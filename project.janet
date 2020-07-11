(declare-project :name "lzo")
(declare-native 
  :name "lzo"
  :source @["lzo/lzo.c"]
  :cflags [;default-cflags "-Os"]
  :ldflags [;default-ldflags "-llzo2"])
