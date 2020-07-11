# janet-lzo

[Janet](https://janet-lang.org/) bindings for the [LZO](http://www.oberhumer.com/opensource/lzo/)
real-time compression library (used by the [`lzop`](https://www.lzop.org/) utility).


## Installation

```
jpm install https://github.com/cellularmitosis/janet-lzo
```


## Usage

`lzo/compress` and `lzo/decompress` operate on and return buffers:

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

## Future work

- allow the user to pass a pre-sized destination buffer to `lzo/decompress`,
to avoid the default _guess-destination-size-and-realloc-as-needed_ loop.
- make `lzo/compress` tolerate a string as input. (currently must be buffer)


## Changelog

- 2020/7/10 fri
  - `#define JANET_LZO_VERSION 0x0003`
  - Move `lzo_init()` call into `JANET_MODULE_ENTRY`
- 2020/7/10 fri (d1cdb80)
  - initial implementation of `lzo/decompress`
- 2020/7/7 tue (7d3ac8b)
  - initial implementation of `lzo/compress`
