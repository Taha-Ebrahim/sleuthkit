/* 
* Author: Taha Ebrahim @Taha-Ebrahim
* Purpose: Tests for test_utils.cpp.
*/
#include <iostream>
#include <cassert>

#include "tsk_tempfile.h"
#include "test_utils.h"
#include <cstdlib>
#include <string>
#include <cstring>

#define SLEUTHKIT_TEST_DATA_DIR "SLEUTHKIT_TEST_DATA_DIR"
#include "catch.hpp"

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

    std::cout << "test_parse_test_line passed.\n";
}

TEST_CASE("compare_file equal") {
    auto f1 = tsk_make_tempfile();
    auto f2 = tsk_make_tempfile();
    
    const char* content = "hello\n";
    fwrite(content, 1, strlen(content), f1);
    fwrite(content, 1, strlen(content), f2);

    REQUIRE(compare_files(f1, f2));
    std::cout << "test_compare_files_equal passed.\n";

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
    std::cout << "test_compare_files_equal passed.\n";
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

    std::ostringstream diff_output;
    std::streambuf* cout_buf = std::cout.rdbuf(diff_output.rdbuf()); // Redirect cout

    print_diff(expected, actual);

    std::cout.rdbuf(cout_buf);  // Reset cout to its original buffer

    std::string diff_result = diff_output.str();

    // Output should contain differences about the lines
    REQUIRE(diff_result.find("Line 3 differs") != std::string::npos);
}


// Test for adjust_tool_path placeholder replacement
TEST_CASE("adjust_tool_path placeholder replacement") {
    const char* original_cmd = "$EXEEXT $DATA_DIR/test $SLEUTHKIT_TEST_DATA_DIR";
    

    std::string result = adjust_tool_path(original_cmd);

    // Also check for $DATA_DIR and $SLEUTHKIT_TEST_DATA_DIR replacements
    REQUIRE(result.find("test/data") != std::string::npos);  
    REQUIRE(result.find("test") != std::string::npos);  
}

// Test for run_test function with a mocked environment
TEST_CASE("run_test with mock environment using tsk_make_tempfile") {
    TestResult result{"test1", "echo hello", 0};
    const char* expected_output = "hello\n";
    const char* expected_error = "";

    // Use tsk_make_tempfile to create temporary files for expected stdout and stderr
    std::string stdout_path, stderr_path;
    FILE* expected_out = tsk_make_named_tempfile(&stdout_path);  // Temporary file for expected stdout
    FILE* expected_err = tsk_make_named_tempfile(&stderr_path);  // Temporary file for expected stderr

    // Check if the files were created successfully
    REQUIRE(expected_out != nullptr);
    REQUIRE(expected_err != nullptr);

    // Write the expected output to the temporary files
    fwrite(expected_output, sizeof(char), strlen(expected_output), expected_out);
    fwrite(expected_error, sizeof(char), strlen(expected_error), expected_err);

    // Reset file pointers to the beginning before reading
    rewind(expected_out);
    rewind(expected_err);

    // Run the test with the temp files
    int status = run_test("echo hello", expected_out, expected_err, 0, result);

    REQUIRE(status == 0);  // Test should pass without errors
    REQUIRE(result.stdout_match);  // Ensure stdout matches
    REQUIRE(result.stderr_match);  // Ensure stderr matches

    // Clean up: Close the temporary files
    fclose(expected_out);
    fclose(expected_err);
}