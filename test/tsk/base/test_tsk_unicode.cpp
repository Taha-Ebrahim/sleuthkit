/*
 * test_tsk_unicode.cpp
 * Author: Taha Ebrahim @Taha-Ebrahim
 * Unit Tests for tsk/base/tsk_unicode.c
 */
#include "tsk/tsk_config.h"
#include "tsk/libtsk.h"
#include "tsk/base/tsk_base_i.h"
#include "tsk/base/tsk_unicode.h"

#include <iostream>
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

TEST_CASE("UTF-16 to UTF-8 decoding with boundary fuzzing", "[utf16to8][fuzz][surrogates]") {
    UTF16 full_input[] = {
        0x0041,               // 'A' (1-byte UTF-8)
        0x00E9,               // 'é' (2-byte UTF-8)
        0x6C34,               // '水' (3-byte UTF-8)
        0xD83D, 0xDE80,       // 🚀 (4-byte UTF-8 surrogate pair)
        0xD83C, 0xDF0D        // 🌍 (4-byte UTF-8 surrogate pair)
    };

    const UTF16* full_end = full_input + sizeof(full_input) / sizeof(UTF16);

    for (int in_len = 0; in_len <= (full_end - full_input); ++in_len) {
        UTF16* input = new UTF16[in_len];
        std::memcpy(input, full_input, in_len * sizeof(UTF16));
        const UTF16* src = input;
        const UTF16* src_end = input + in_len;

        // Estimate worst-case output size (4 bytes per UTF-16 code unit)
        int max_utf8_len = 4 * in_len;

        for (int out_len = std::max(0, max_utf8_len - 2); out_len <= max_utf8_len + 2; ++out_len) {
            UTF8* output = new UTF8[out_len];
            UTF8* tgt = output;
            UTF8* tgt_end = output + out_len;

            TSKConversionResult res = tsk_UTF16toUTF8(
                TSK_LIT_ENDIAN,
                &src,
                src_end,
                &tgt,
                tgt_end,
                TSKstrictConversion
            );

            if (res != TSKconversionOK &&
                res != TSKtargetExhausted &&
                res != TSKsourceIllegal &&
                res != TSKsourceExhausted)
            {
                FAIL("Unexpected conversion result: " << res
                    << " (in_len=" << in_len
                    << ", out_len=" << out_len << ")");
            }

            delete[] output;
        }

        delete[] input;
    }
}

TEST_CASE("UTF-8 to UTF-16 decoding with boundary fuzzing", "[utf8to16][fuzz]") {
    // UTF-8 string: "Aé水🚀🌍"
    const UTF8 full_input[] = {
        0x41,                         // 'A' (1 byte)
        0xC3, 0xA9,                   // 'é' (2 bytes)
        0xE6, 0xB0, 0xB4,             // '水' (3 bytes)
        0xF0, 0x9F, 0x9A, 0x80,       // 🚀 (4 bytes)
        0xF0, 0x9F, 0x8C, 0x8D        // 🌍 (4 bytes)
    };
    const size_t input_len = sizeof(full_input);
    
    for (size_t in_len = 0; in_len <= input_len; ++in_len) {
        UTF8* input = new UTF8[in_len];
        std::memcpy(input, full_input, in_len);
        const UTF8* src = input;
        const UTF8* src_end = input + in_len;

        // Estimate max number of UTF-16 code units (max 2 per character)
        int max_utf16_len = 2 * in_len;

        for (int out_len = std::max(0, max_utf16_len - 2); out_len <= max_utf16_len + 2; ++out_len) {
            UTF16* output = new UTF16[out_len];
            UTF16* tgt = output;
            UTF16* tgt_end = output + out_len;

            TSKConversionResult res = tsk_UTF8toUTF16(
                &src,
                src_end,
                &tgt,
                tgt_end,
                TSKstrictConversion
            );

            std::cout << "Input: " << in_len << ", Output: " << out_len << ", Result: " << res << '\n';


            if (res != TSKconversionOK &&
                res != TSKtargetExhausted &&
                res != TSKsourceIllegal &&
                res != TSKsourceExhausted)
            {
                FAIL("Unexpected conversion result: " << res
                    << " (in_len=" << in_len
                    << ", out_len=" << out_len << ")");
            }

            delete[] output;
        }

        delete[] input;
    }
}

TEST_CASE("UTF-16 to UTF-8 local order: valid basic conversion", "[utf16to8][lclorder]") {
    UTF16 input[] = { 'T', 'e', 's', 't', 0x20AC };  // "Test€"
    const UTF16 *src = input;
    const UTF16 *src_end = input + 5;

    UTF8 output[16];
    UTF8 *tgt = output;
    UTF8 *tgt_end = output + 16;

    TSKConversionResult res = tsk_UTF16toUTF8_lclorder(&src, src_end, &tgt, tgt_end, TSKstrictConversion);
    REQUIRE(res == TSKconversionOK);
}

#ifndef __MINGW32__
TEST_CASE("UTF-16W to UTF-8 local order: emoji surrogate", "[utf16to8][wchar]") {
    const wchar_t input[] = { 0xD83D, 0xDE80 };  // 🚀 = \U0001F680
    const wchar_t *src = input;
    const wchar_t *src_end = input + 2;

    UTF8 output[8];
    UTF8 *tgt = output;
    UTF8 *tgt_end = output + 8;

    TSKConversionResult res = tsk_UTF16WtoUTF8_lclorder(&src, src_end, &tgt, tgt_end, TSKstrictConversion);
    REQUIRE(res == TSKconversionOK);
}
#endif

TEST_CASE("tsk_cleanupUTF8 replaces invalid UTF-8 with replacement char", "[cleanup][utf8]") {
    char input[] = { (char)0xC0, (char)0xAF, 'X', '\0' }; // overlong encoding of '/'
    tsk_cleanupUTF8(input, '?');
    REQUIRE(input[0] == '?');
    REQUIRE(input[1] == '?');
    REQUIRE(input[2] == 'X');
}

TEST_CASE("tsk_cleanupUTF16 replaces unpaired surrogate with replacement", "[cleanup][utf16]") {
    wchar_t input[] = { 0xD800, 0x0041 }; // unpaired high surrogate + 'A'
    tsk_cleanupUTF16(TSK_LIT_ENDIAN, input, 2, L'?');
    REQUIRE(input[0] == L'?');
    REQUIRE(input[1] == L'A');
}

TEST_CASE("tsk_isLegalUTF8Sequence detects legal and illegal UTF-8", "[validation][utf8]") {
    UTF8 valid[] = { 0xE2, 0x82, 0xAC }; // €
    UTF8 *valid_end = valid + 3;

    UTF8 invalid[] = { 0xC0, 0xAF }; // overlong '/'
    UTF8 *invalid_end = invalid + 2;

    REQUIRE(tsk_isLegalUTF8Sequence(valid, valid_end) == true);
    REQUIRE(tsk_isLegalUTF8Sequence(invalid, invalid_end) == false);
}

TEST_CASE("UTF-8 to UTF-16: incomplete multibyte sequence (sourceExhausted)", "[utf8to16][exhausted]") {
    UTF8 input[] = { 0xE2, 0x82 }; // partial €
    const UTF8 *src = input;
    const UTF8 *src_end = input + sizeof(input);

    UTF16 output[4];
    UTF16 *tgt = output;
    UTF16 *tgt_end = output + 4;

    TSKConversionResult res = tsk_UTF8toUTF16(&src, src_end, &tgt, tgt_end, TSKstrictConversion);
    REQUIRE(res == TSKsourceExhausted);
}

TEST_CASE("UTF-8 to UTF-16: decode character > 0x10FFFF should fail in strict mode", "[utf8to16][invalid]") {
    UTF8 input[] = { 0xF4, 0x90, 0x80, 0x80 }; // > U+10FFFF
    const UTF8 *src = input;
    const UTF8 *src_end = input + 4;

    UTF16 output[4];
    UTF16 *tgt = output;
    UTF16 *tgt_end = output + 4;

    TSKConversionResult res = tsk_UTF8toUTF16(&src, src_end, &tgt, tgt_end, TSKstrictConversion);
    REQUIRE(res == TSKsourceIllegal);
}