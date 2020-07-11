# janet-lzo

Janet bindings for the LZO real-time compression library.

# Installation

```
jpm install https://github.com/cellularmitosis/janet-lzo
```

# Usage

`lzo/compress` and `lzo/decompress` operate on buffers:

```
> (import lzo)
nil

> (lzo/compress @"hello")
@"\x16hello\x11\0\0"

> (lzo/decompress (lzo/compress @"hello"))
@"hello"

> (lzo/compress (buffer (string/repeat "hello" 50)))
@"\x02hello \xC3\x10\0\x0Elohellohellohello\x11\0\0"

> (lzo/decompress (lzo/compress (buffer (string/repeat "hello" 50))))
@"hellohellohellohellohellohellohellohellohellohellohellohellohellohellohellohel
lohellohellohellohellohellohellohellohellohellohellohellohellohellohellohellohel
lohellohellohellohellohellohellohellohellohellohellohellohellohellohellohellohel
lohellohello"
```

This means it can also be used with `marshal` and `unmarshal`:

```
> (lzo/compress (marshal {:a 1 :b 2}))
@"\e\xD5\x02\xD0\x01a\x01\xD0\x01b\x02\x11\0\0"

> (unmarshal (lzo/decompress (lzo/compress (marshal {:a 1 :b 2}))))
{:a 1 :b 2}
```
