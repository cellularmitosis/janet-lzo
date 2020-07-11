(declare-project
  :name "lzo"
  :author "Jason Pepas"
  :license "MIT"
  :url "https://github.com/cellularmitosis/janet-lzo"
  :repo "git+https://github.com/cellularmitosis/janet-lzo.git")

(declare-native 
  :name "lzo"
  :source @["lzo.c"]
  :cflags [;default-cflags "-Os"]
  :ldflags [;default-ldflags "-llzo2"])
