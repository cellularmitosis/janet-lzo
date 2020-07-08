(declare-project :name "lzo")
(declare-native 
  :name "lzo"
  :source @["lzo.c"]
  :cflags [;default-cflags "-Os" "-Wno-unused-parameter"]
  :ldflags [;default-ldflags "-llzo2"])
