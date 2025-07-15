/*
 * test_tsk_unicode.cpp
 * Author: Taha Ebrahim @Taha-Ebrahim
 * Unit Tests for tsk/base/tsk_unicode.c
 */
#include "tsk/tsk_config.h"
#include "tsk/libtsk.h"
#include "tsk/base/tsk_base_i.h"
#include "tsk/base/tsk_unicode.h"


#include <cstring>

#include "catch.hpp"

TEST_CASE("UTF-8 to UTF-16 happy path", "[utf8to16]") {
    const UTF8 *src = (const UTF8 *)"hello €世界";
    const UTF8 *src_end = src + strlen((const char *)src);

    UTF16 output[64];
    UTF16 *out_start = output;
    UTF16 *out_end = output + 64;

    TSKConversionResult res = tsk_UTF8toUTF16(&src, src_end, &out_start, out_end, TSKstrictConversion);
    REQUIRE(res == TSKconversionOK);
}

TEST_CASE("UTF-16 to UTF-8 happy path", "[utf16to8]") {
    UTF16 input[] = { 'h', 'e', 'l', 'l', 'o', 0x20AC, 0x4E16, 0x754C };
    const UTF16 *src = input;
    const UTF16 *src_end = input + sizeof(input) / sizeof(input[0]);

    UTF8 output[64];
    UTF8 *out_start = output;
    UTF8 *out_end = output + 64;

    TSKConversionResult res = tsk_UTF16toUTF8(TSK_LIT_ENDIAN, &src, src_end, &out_start, out_end, TSKstrictConversion);
    REQUIRE(res == TSKconversionOK);
}

TEST_CASE("UTF-16 to UTF-8 fails when buffer is 1 byte short", "[utf16to8][shortbuffer]") {
    UTF16 input[] = { 'A', 0x20AC };
    const UTF16 *src = input;
    const UTF16 *src_end = input + 2;

    UTF8 output[3]; // only enough for 'A' + partial €
    UTF8 *out_start = output;
    UTF8 *out_end = output + 3;

    TSKConversionResult res = tsk_UTF16toUTF8(TSK_LIT_ENDIAN, &src, src_end, &out_start, out_end, TSKstrictConversion);
    REQUIRE(res == TSKtargetExhausted);
}

TEST_CASE("UTF-16 surrogate pair is correctly converted to UTF-8", "[utf16to8][surrogate]") {
    UTF16 surrogate_pair[] = { 0xD852, 0xDF62 }; // 🚀 = U+1F680

    const UTF16* src = surrogate_pair;
    const UTF16* src_end = surrogate_pair + 2;

    UTF8 output[5] = {}; // 4 bytes + null terminator (optional for testing convenience)
    UTF8* tgt = output;
    UTF8* tgt_end = output + 4;

    TSKConversionResult res = tsk_UTF16toUTF8(
        TSK_LIT_ENDIAN,
        &src,
        src_end,
        &tgt,
        tgt_end,
        TSKlenientConversion 
    );

    REQUIRE(res == TSKconversionOK);
}

TEST_CASE("UTF-16 surrogate pair fails with 1-3 byte output buffers", "[utf16to8][surrogate][targetExhausted]") {
    UTF16 surrogate_pair[] = { 0xD852, 0xDF62 }; // 𤭢 (U+24B62), valid surrogate pair

    for (int buffer_size = 1; buffer_size <= 3; ++buffer_size) {
        INFO("Testing with output buffer of " << buffer_size << " bytes");

        const UTF16* src = surrogate_pair;
        const UTF16* src_end = surrogate_pair + 2;

        UTF8 output[4] = {};  // Large enough array, but we simulate smaller buffers
        UTF8* tgt = output;
        UTF8* tgt_end = output + buffer_size;

        TSKConversionResult res = tsk_UTF16toUTF8(
            TSK_LIT_ENDIAN,
            &src,
            src_end,
            &tgt,
            tgt_end,
            TSKlenientConversion
        );

        REQUIRE(res == TSKtargetExhausted);
    }
}

TEST_CASE("UTF-8 to UTF-16 detects malformed sequence", "[utf8to16][malformed]") {
    UTF8 input[] = { 0xC0, 0xAF };  // overlong encoding
    const UTF8 *src = input;
    const UTF8 *src_end = input + sizeof(input);

    UTF16 output[4];
    UTF16 *out_start = output;
    UTF16 *out_end = output + 4;

    TSKConversionResult res = tsk_UTF8toUTF16(&src, src_end, &out_start, out_end, TSKstrictConversion);
    REQUIRE(res == TSKsourceIllegal);
}

TEST_CASE("UTF-16 to UTF-8 fails on unpaired high surrogate", "[utf16to8][unpaired]") {
    UTF16 input[] = { 0xD83D };  // unpaired high surrogate
    const UTF16 *src = input;
    const UTF16 *src_end = input + 1;

    UTF8 output[8];
    UTF8 *out_start = output;
    UTF8 *out_end = output + 8;

    TSKConversionResult res = tsk_UTF16toUTF8(TSK_LIT_ENDIAN, &src, src_end, &out_start, out_end, TSKstrictConversion);
    REQUIRE(res == TSKsourceExhausted);
}

TEST_CASE("UTF-16 to UTF-8 fails on unpaired low surrogate", "[utf16to8][unpaired]") {
    UTF16 input[] = { 0xDC00 };  // unpaired low surrogate
    const UTF16 *src = input;
    const UTF16 *src_end = input + 1;

    UTF8 output[8];
    UTF8 *out_start = output;
    UTF8 *out_end = output + 8;

    TSKConversionResult res = tsk_UTF16toUTF8(TSK_LIT_ENDIAN, &src, src_end, &out_start, out_end, TSKstrictConversion);
    REQUIRE(res == TSKsourceIllegal);
}

TEST_CASE("UTF-8 to UTF-16 fails when target buffer is too small", "[utf8to16][targetExhausted]") {
    const UTF8 *src = (const UTF8 *)"€";  // U+20AC, needs 1 UTF-16 unit
    const UTF8 *src_end = src + strlen((const char *)src);

    UTF16 output[0];  // no room at all
    UTF16 *out_start = output;
    UTF16 *out_end = output;

    TSKConversionResult res = tsk_UTF8toUTF16(&src, src_end, &out_start, out_end, TSKstrictConversion);
    REQUIRE(res == TSKtargetExhausted);
}


