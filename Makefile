test: build/lzo.a
	janet -e '(import build/lzo :as lzo) (lzo/compress @"hello")'

build/lzo.a: lzo.c
	jpm build

repl: build/lzo.a
	janet -e '(import build/lzo :as lzo)' -r

clean:
	rm -rf build

.PHONY: test repl clean
