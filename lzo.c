// lzo.c: Janet bindings for the lzo realtime compression library.
// Copyright (c) 2020 Jason Pepas.
// Released under the terms of the MIT license.
// See https://opensource.org/licenses/MIT

// See also:
// http://www.oberhumer.com/opensource/lzo/
// https://janet-lang.org/capi/index.html
// lzo-2.XX/doc/LZOAPI.TXT

#include <janet.h>
#include <lzo/lzo1x.h>  // LZO1X is the most commonly used algo.

#if LZO_VERSION < 0x2000
#warning "janet-lzo was not written for lzo versions prior to 2.00.  Good luck!"
#endif

#if LZO_VERSION >= 0x20a0
#warning "Hello there, future hacker!  The janet-lzo module was written against \
lzo 2.10, but your version is more recent.  Most likely this is fine, but please \
submit a github issue to have janet-lzo updated for the latest release of lzo. \
See https://github.com/cellularmitosis/janet-lzo/issues"
#endif

// gcc-specific branch-prediction optimizations:
#if defined(__GNUC__) && !defined(likely)
#define unlikely(x)     __builtin_expect(!!(x), 0)
#else
#define unlikely(x)     (x)
#endif

// lzo error codes as panic strings.
static const char* lzo_emsg_error = "lzo failed with LZO_E_ERROR";
static const char* lzo_emsg_out_of_memory = "lzo failed with LZO_E_OUT_OF_MEMORY";
static const char* lzo_emsg_not_compressible = "lzo failed with LZO_E_NOT_COMPRESSIBLE";
static const char* lzo_emsg_input_overrun = "lzo failed with LZO_E_INPUT_OVERRUN";
static const char* lzo_emsg_output_overrun = "lzo failed with LZO_E_OUTPUT_OVERRUN";
static const char* lzo_emsg_lookbehind_overrun = "lzo failed with LZO_E_LOOKBEHIND_OVERRUN";
static const char* lzo_emsg_eof_not_found = "lzo failed with LZO_E_EOF_NOT_FOUND";
static const char* lzo_emsg_input_not_consumed = "lzo failed with LZO_E_INPUT_NOT_CONSUMED";
static const char* lzo_emsg_not_yet_implemented = "lzo failed with LZO_E_NOT_YET_IMPLEMENTED";
static const char* lzo_emsg_invalid_argument = "lzo failed with LZO_E_INVALID_ARGUMENT";
static const char* lzo_emsg_invalid_alignment = "lzo failed with LZO_E_INVALID_ALIGNMENT";
static const char* lzo_emsg_output_not_consumed = "lzo failed with LZO_E_OUTPUT_NOT_CONSUMED";
static const char* lzo_emsg_internal_error = "lzo failed with LZO_E_INTERNAL_ERROR";
static const char* lzo_emsg_unknown = "lzo failed with an error code unknown as of lzo-2.10";

static const char* lzo_err_as_string(int err) {
    switch (err) {
        case LZO_E_ERROR:
            return lzo_emsg_error;
        case LZO_E_OUT_OF_MEMORY:
            return lzo_emsg_out_of_memory;
        case LZO_E_NOT_COMPRESSIBLE:
            return lzo_emsg_not_compressible;
        case LZO_E_INPUT_OVERRUN:
            return lzo_emsg_input_overrun;
        case LZO_E_OUTPUT_OVERRUN:
            return lzo_emsg_output_overrun;
        case LZO_E_LOOKBEHIND_OVERRUN:
            return lzo_emsg_lookbehind_overrun;
        case LZO_E_EOF_NOT_FOUND:
            return lzo_emsg_eof_not_found;
        case LZO_E_INPUT_NOT_CONSUMED:
            return lzo_emsg_input_not_consumed;
        case LZO_E_NOT_YET_IMPLEMENTED:
            return lzo_emsg_not_yet_implemented;
#if LZO_VERSION >= 0x2050
        case LZO_E_INVALID_ARGUMENT:
            return lzo_emsg_invalid_argument;
#endif
#if LZO_VERSION >= 0x2070
        case LZO_E_INVALID_ALIGNMENT:
            return lzo_emsg_invalid_alignment;
        case LZO_E_OUTPUT_NOT_CONSUMED:
            return lzo_emsg_output_not_consumed;
        case LZO_E_INTERNAL_ERROR:
            return lzo_emsg_internal_error;
#endif
        default:
            return lzo_emsg_unknown;
    }
}

// janet lzo/compress.
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
    // 128KB on 64-bit, 64KB on 32-bit systems.
    lzo_uint32_t wrkmem[LZO1X_1_MEM_COMPRESS];

    // compress the buffer.
    const lzo_bytep src = (lzo_bytep)(jbuff->data);
    lzo_uint src_len = (lzo_uint)(jbuff->count);
    lzo_bytep dest = (lzo_bytep)(jbuff_out->data);
    lzo_uint dest_len = (lzo_uint)(jbuff_out->capacity);
    int err = lzo1x_1_compress(src, src_len, dest, &dest_len, wrkmem);
    if (unlikely( err != LZO_E_OK )) {
        // Note: according to lzo-2.XX/doc/LZOAPI.TXT, lzo1x_1_compress always
        // returns LZO_E_OK (errors only occur during decompression).
        janet_panicf(lzo_err_as_string(err));
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

// make the c functions visible to janet.
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
