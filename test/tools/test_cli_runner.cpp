/* 
* Author: Taha Ebrahim @Taha-Ebrahim
* Purpose: Tests for test_utils.cpp.
*/
#include <iostream>
#include <cassert>

#include "tsk_tempfile.h"
#include "test_utils.h"
#include "tsk/base/tsk_printf.h"
#include <cstdlib>
#include <string>
#include <cstring>

#define SLEUTHKIT_TEST_DATA_DIR "SLEUTHKIT_TEST_DATA_DIR"
#include "catch.hpp"

FILE* cli_temp_output = tsk_make_tempfile();
FILE* cli_temp_err = tsk_make_tempfile();

FILE* cli_old_output = tsk_set_printf_fd(cli_temp_output);
FILE* cli_old_err = tsk_set_stderr_fd(cli_temp_err);

TEST_CASE("parse_test_line") {
    std::string line = "t1|echo hi|out.txt|0";
    std::string id, cmd, output, error;
    int exit = -1;

    bool ok = parse_test_line(line, id, cmd, output, exit, error);
    REQUIRE(ok);
    REQUIRE(id == "t1");
    REQUIRE(cmd == "echo hi");
    REQUIRE(output == "out.txt");
    REQUIRE(exit == 0);

}

TEST_CASE("compare_file equal") {
    auto f1 = tsk_make_tempfile();
    auto f2 = tsk_make_tempfile();
    
    const char* content = "hello\n";
    fwrite(content, 1, strlen(content), f1);
    fwrite(content, 1, strlen(content), f2);

    REQUIRE(compare_files(f1, f2));

    fclose(f1);
    fclose(f2);
}

TEST_CASE("compare_file unequal") {
    auto f1 = tsk_make_tempfile();
    auto f2 = tsk_make_tempfile();
    
    const char* content = "hello\n";
    const char* content2 = "world\n";
    
    fwrite(content, 1, strlen(content), f1);
    fwrite(content2, 1, strlen(content2), f2);

    REQUIRE(!compare_files(f1, f2));
    fclose(f1);
    fclose(f2);
}

TEST_CASE("readfile") {
    auto f1 = tsk_make_tempfile();
    
    const char* content = "hello\n";
    fwrite(content, 1, strlen(content), f1);

    assert(read_file(f1) == "hello\n");
    fclose(f1);
}

/// Test for print_diff when lines differ
TEST_CASE("print_diff when lines differ") {
    std::string expected = "Line 1\nLine 2\nLine 3\n";
    std::string actual = "Line 1\nLine 2\nLine 4\n";

    // Clear and rewind the tsk_printf FILE before test
    fflush(cli_old_output);
    fseek(cli_old_output, 0, SEEK_SET);

    // Call the function, which uses tsk_printf internally
    print_diff(expected, actual);

    // Read what tsk_printf wrote to the temp file
    std::string diff_result = read_file(cli_old_output);

    // Output should mention a line difference
    REQUIRE(diff_result.find("differs") != std::string::npos);
    REQUIRE(diff_result.find("Expected:") != std::string::npos);
    REQUIRE(diff_result.find("Actual  :") != std::string::npos);
}


// Test for adjust_tool_path placeholder replacement
TEST_CASE("adjust_tool_path placeholder replacement") {
    const char* original_cmd = "$EXEEXT $DATA_DIR/test $SLEUTHKIT_TEST_DATA_DIR";
    

    std::string result = adjust_tool_path(original_cmd);

    // Also check for $DATA_DIR and $SLEUTHKIT_TEST_DATA_DIR replacements
    REQUIRE(result.find("test/data") != std::string::npos);  
    REQUIRE(result.find("test") != std::string::npos);  
}

struct CleanupCliTempFiles {
    ~CleanupCliTempFiles() {
        tsk_set_printf_fd(cli_old_output);
        tsk_set_stderr_fd(cli_old_err);
        if (cli_temp_output) fclose(cli_temp_output);
        if (cli_temp_err) fclose(cli_temp_err);
    }
};

CleanupCliTempFiles cli_cleanup_guard;
