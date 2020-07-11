#!/usr/bin/env janet

# this script does some basic benchmarking to determine compression speed
# and compression ratio of lzop vs gzip, at all levels of compression.

(defn split-words
  "Split text up into tab-delimited words."
  [text]
  (def words (string/split "\t" text))
  (def non-empty-words (filter |(> (length $) 0) words))
  non-empty-words)

(defn parse-bash-timeval
  "Parse a bash 'time' timeval (e.g. '0m0.020s') into a double."
  [word]
  (def mins (scan-number
    (0 (string/split "m" word))))
  (def secs (scan-number
    (string/trimr (1 (string/split "m" word)) "s")))
  (def total-time (+ (* mins 60) secs))
  total-time)

(defn parse-bash-time-output
  "Parse the real/user/sys output of a bash 'time' command."
  [text]
  (def lines (string/split "\n" text))
  (def stats @{})
  (each line lines
    (def words (split-words line))
    (def word0 (get words 0))
    (cond
      (= word0 "real")
        (put stats :real (parse-bash-timeval (1 words)))
      (= word0 "user")
        (put stats :user (parse-bash-timeval (1 words)))
      (= word0 "sys")
        (put stats :sys (parse-bash-timeval (1 words)))))
  stats)

(defn make-z-cmdline
  "Create a compression command line (string)."
  [zcmd fname level]
  (def template "bash -c 'time (cat %s | %s %s >/dev/null) 2>/tmp/stderr'")
  (def cmdline (string/format template fname zcmd level))
  cmdline)

(defn run-z-cmd
  "Run a z command once and parse the bash time output."
  [zcmd fname level]
  (def cmd (make-z-cmdline zcmd fname level))
  (file/write stderr (string "running: " cmd "\n"))
  (file/flush stderr)
  (def status (os/shell cmd))
  (assert (= status 0))
  (def output (slurp "/tmp/stderr"))
  (os/rm "/tmp/stderr")
  (def stats (parse-bash-time-output output))
  stats)

(defn averaged-bench-results
  "Return the averages of many real/user/sys results."
  [results]
  {:real (mean (map (fn [ds] (ds :real)) results))
   :user (mean (map (fn [ds] (ds :user)) results))
   :sys  (mean (map (fn [ds] (ds :sys)) results))  })

(defn bench-z-cmd
  "Run a z command three times and return the averaged results."
  [zcmd fname level]
  # run once and throw away results to prime fs cache, etc.
  (run-z-cmd zcmd fname level)
  # run three times and average the results.
  (def results (mapcat (fn [_] (run-z-cmd zcmd fname level)) [1 2 3]))
  (def avg-results (averaged-bench-results results))
  avg-results)

(defn bench-zcmd-all-levels
  "Run a z command at compression levels 1 through 9."
  [zcmd fname]
  (def results @{})
  (cond
    (= zcmd "cat")
      (do
        (def level "")
        (def avgd-results (bench-z-cmd zcmd fname level))
        (put results [zcmd fname level] avgd-results))
    :else
      (each level ["-1" "-2" "-3" "-4" "-5" "-6" "-7" "-8" "-9"]
        (def avgd-results (bench-z-cmd zcmd fname level))
        (put results [zcmd fname level] avgd-results)))
  results)

(defn bench-ratios
  "Determine the compressor's compression ratios at all compression levels."
  [zcmd fname]
  (def orig-size (os/stat fname :size))
  (def ratios @{})
  (each level ["-1" "-2" "-3" "-4" "-5" "-6" "-7" "-8" "-9"]
    (def cmd (string/format "cat %s | %s %s > /tmp/out.z" fname zcmd level))
    (print "running: " cmd)
    (def status (os/shell cmd))
    (assert (= status 0))
    (def size (os/stat "/tmp/out.z" :size))
    (def ratio (/ 1 (/ size orig-size)))
    (put ratios [zcmd level] ratio)
    (os/rm "/tmp/out.z"))
  ratios)

(defn main [x]
  (def input-file "linux-5.7.7.tar.4MB")
  # determine compression speed
  (def results @{})
  (merge-into results (bench-zcmd-all-levels "cat" input-file))
  (merge-into results (bench-zcmd-all-levels "lzop" input-file))
  (merge-into results (bench-zcmd-all-levels "gzip" input-file))
  (print (string/format "%M" results))
  (print "saving marshalled results to /tmp/results.mjdn")
  (spit "/tmp/results.mjdn" (marshal results))
  # determine compression ratios
  (def ratios @{})
  (merge-into ratios (bench-ratios "lzop" input-file))
  (merge-into ratios (bench-ratios "gzip" input-file))
  (print (string/format "%M" ratios))
  (print "saving marshalled ratios to /tmp/ratios.mjdn")
  (spit "/tmp/ratios.mjdn" (marshal ratios)))
