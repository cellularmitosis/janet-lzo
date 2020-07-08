#include <janet.h>
#include <lzo/lzo1x.h>

// lzo.c: Janet bindings for the lzo realtime compression library.
// Copyright (c) 2020 Jason Pepas.
// Released under the terms of the MIT license.
// See https://opensource.org/licenses/MIT

// See also:
// https://janet-lang.org/capi/index.html
// http://www.oberhumer.com/opensource/lzo/


// gcc-specific branch-prediction optimizations:
#if defined(__GNUC__) && !defined(likely)
#define unlikely(x)     __builtin_expect(!!(x), 0)
#else
#define unlikely(x)     (x)
#endif


static Janet cfun_lzo_compress(int32_t argc, Janet *argv) {
    // we expect exactly 1 arg.
    janet_fixarity(argc, 1);

    // the arg is a buffer.
    Janet x = argv[0];
    if (unlikely( !janet_checktype(x, JANET_BUFFER) )) {
        janet_panicf("expected buffer, got %t: %v", x, x);
    }
    JanetBuffer* jbuff = janet_unwrap_buffer(x);

    // the destination buffer.
    // minilzo/testmini.c uses OUT_LEN = (IN_LEN + IN_LEN / 16 + 64 + 3).
    int32_t capacity = jbuff->count + ((jbuff->count) / 16) + 64 + 3;
    JanetBuffer* jbuff_out = janet_buffer(capacity);

    // the working memory required by lzo during compression.
    lzo_uint32_t wrkmem[LZO1X_1_MEM_COMPRESS];

    // compress the buffer.
    const lzo_bytep src = (lzo_bytep)(jbuff->data);
    lzo_uint src_len = (lzo_uint)(jbuff->count);
    lzo_bytep dest = (lzo_bytep)(jbuff_out->data);
    lzo_uint dest_len = (lzo_uint)(jbuff_out->capacity);
    int err = lzo1x_1_compress(src, src_len, dest, &dest_len, wrkmem);
    if (unlikely( err != LZO_E_OK )) {
        switch (err) {
            case LZO_E_ERROR:
                janet_panicf("lzo failed with LZO_E_ERROR (%d)", err);
            case LZO_E_OUT_OF_MEMORY:
                janet_panicf("lzo failed with LZO_E_OUT_OF_MEMORY (%d)", err);
            case LZO_E_NOT_COMPRESSIBLE:
                janet_panicf("lzo failed with LZO_E_NOT_COMPRESSIBLE (%d)", err);
            case LZO_E_INPUT_OVERRUN:
                janet_panicf("lzo failed with LZO_E_INPUT_OVERRUN (%d)", err);
            case LZO_E_OUTPUT_OVERRUN:
                janet_panicf("lzo failed with LZO_E_OUTPUT_OVERRUN (%d)", err);
            case LZO_E_LOOKBEHIND_OVERRUN:
                janet_panicf("lzo failed with LZO_E_LOOKBEHIND_OVERRUN (%d)", err);
            case LZO_E_EOF_NOT_FOUND:
                janet_panicf("lzo failed with LZO_E_EOF_NOT_FOUND (%d)", err);
            case LZO_E_INPUT_NOT_CONSUMED:
                janet_panicf("lzo failed with LZO_E_INPUT_NOT_CONSUMED (%d)", err);
            case LZO_E_NOT_YET_IMPLEMENTED:
                janet_panicf("lzo failed with LZO_E_NOT_YET_IMPLEMENTED (%d)", err);
            case LZO_E_INVALID_ARGUMENT:
                janet_panicf("lzo failed with LZO_E_INVALID_ARGUMENT (%d)", err);
            case LZO_E_INVALID_ALIGNMENT:
                janet_panicf("lzo failed with LZO_E_INVALID_ALIGNMENT (%d)", err);
            case LZO_E_OUTPUT_NOT_CONSUMED:
                janet_panicf("lzo failed with LZO_E_OUTPUT_NOT_CONSUMED (%d)", err);
            case LZO_E_INTERNAL_ERROR:
                janet_panicf("lzo failed with LZO_E_INTERNAL_ERROR (%d)", err);
            default:
                janet_panicf("lzo failed with unknown error code %d", err);
        }
    } else {
        jbuff_out->count = (int32_t)dest_len;
    }

    // return the compressed buffer.
    return janet_wrap_buffer(jbuff_out);
}

// TODO
// static Janet cfun_lzo_compress(int32_t argc, Janet *argv) {
//
// }

static const JanetReg cfuns[] = {
    {
        "compress", cfun_lzo_compress,
        "(lzo/compress (marshal ds))\n\nCompress a buffer.\nReturns a buffer."
    },
    // {
    //     "decompress", cfun_lzo_decompress,
    //     "(unmarshal (lzo/decompress zds))\n\nDecompress a buffer.\nReturns a buffer."
    // },
    {NULL, NULL, NULL}
};

JANET_MODULE_ENTRY(JanetTable *env) {
    janet_cfuns(env, "lzo", cfuns);
}
