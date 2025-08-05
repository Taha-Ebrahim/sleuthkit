/*
 * test_tsk_unicode.cpp
 * Author: Taha Ebrahim @Taha-Ebrahim
 * Unit Tests for tsk/base/tsk_unicode.c
 */
#include "tsk/tsk_config.h"
#include "tsk/libtsk.h"
#include "tsk/base/tsk_base_i.h"
#include "tsk/base/tsk_unicode.h"
#include "tsk/fs/tsk_fs_i.h"
#include "tsk/fs/tsk_ntfs.h"

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
    // The correct UTF-16 surrogate pair for U+1F60E is D83D DE0E.
    // We'll create the UTF-16 values directly.
    UTF16 surrogate_pair[] = { 0xD83D, 0xDE0E };

    const UTF16* src = surrogate_pair;
    const UTF16* src_end = surrogate_pair + 2;

    // The UTF-8 representation of U+1F60E is F0 9F 98 8E.
    UTF8 output[5] = {0};
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

    // Assert that the conversion was successful.
    REQUIRE(res == TSKconversionOK);
    
    // Verify the output UTF-8 sequence.
    REQUIRE(output[0] == 0xF0);
    REQUIRE(output[1] == 0x9F);
    REQUIRE(output[2] == 0x98);
    REQUIRE(output[3] == 0x8E);
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

TEST_CASE("tsk_safeUTF16toUTF8 prevents out-of-bounds read", "[unicode][safe]") {
    // 1. Set up a small, fixed-size source buffer.
    // The buffer is 4 bytes and can hold 2 UTF-16 characters.
    const size_t source_buffer_len = 4;
    uint8_t source_buffer[source_buffer_len];
    // Fill the buffer with valid UTF-16 characters: "AB"
    source_buffer[0] = 'A'; source_buffer[1] = 0;
    source_buffer[2] = 'B'; source_buffer[3] = 0;
    
    // 2. Set up the target buffer.
    const size_t target_buffer_len = 10;
    UTF8 target_buffer[target_buffer_len] = {0};
    UTF8 *target_ptr = target_buffer;
    UTF8 *target_end_ptr = target_buffer + target_buffer_len;

    // 3. Deliberately provide a malformed number_of_characters.
    // We claim there are 3 characters, but the buffer only holds 2.
    const size_t malformed_num_chars = 3;

    // 4. Call the fixed function with the malformed input.
    // The function is expected to correct the input before any unsafe access occurs.
    SECTION("Function call with malformed input") {
        REQUIRE_NOTHROW(tsk_safeUTF16toUTF8(TSK_LIT_ENDIAN, source_buffer, source_buffer_len,
                                            malformed_num_chars, &target_ptr, target_end_ptr,
                                            TSKlenientConversion));

        // 5. Verify the function's behavior.
        // The function should return TSKconversionOK as it safely converted the available data.
        REQUIRE(tsk_safeUTF16toUTF8(TSK_LIT_ENDIAN, source_buffer, source_buffer_len,
                                    malformed_num_chars, &target_ptr, target_end_ptr,
                                    TSKlenientConversion) == TSKconversionOK);
    }
}