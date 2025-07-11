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