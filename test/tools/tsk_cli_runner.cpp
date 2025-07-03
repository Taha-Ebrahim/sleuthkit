#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cstdlib>
#include <chrono>
#include <cstdio>
#include <iomanip>

#include "tsk_tempfile.h"
#include "tsk_cli_runner.h"

bool parse_test_line(const std::string& line,
                     std::string& id,
                     std::string& cmd,
                     std::string& expected_file,
                     int& expected_exit)
{
    size_t pos1 = line.find('|');
    size_t pos2 = line.find('|', pos1 + 1);
    size_t pos3 = line.find('|', pos2 + 1);
    if (pos1 == std::string::npos || pos2 == std::string::npos || pos3 == std::string::npos)
        return false;

    id = line.substr(0, pos1);
    cmd = line.substr(pos1 + 1, pos2 - pos1 - 1);
    expected_file = line.substr(pos2 + 1, pos3 - pos2 - 1);
    expected_exit = std::stoi(line.substr(pos3 + 1));
    return true;
}

std::string read_file(FILE* file) {
    fflush(file);               // flush stdout buffers
    fseek(file, 0, SEEK_SET);   // rewind to start

    // Read file contents into a buffer
    char buffer[4096] = {0};
    fread(buffer, 1, sizeof(buffer) - 1, file);
    fclose(file);

    // Convert to std::string for assertion
    std::string output(buffer);
    return output;
}

bool compare_files(FILE* expected, FILE* actual) {
    std::cout << "Expected File: " << read_file(expected) << '\n';
    std::cout << "Output: " << read_file(expected);
    return read_file(expected) == read_file(actual);
}
void print_diff(const std::string& expected, const std::string& actual) {
    std::istringstream expected_stream(expected);
    std::istringstream actual_stream(actual);

    std::string expected_line, actual_line;
    int line_num = 1;
    while (std::getline(expected_stream, expected_line) &&
           std::getline(actual_stream, actual_line)) {
        if (expected_line != actual_line) {
            std::cout << "  Line " << line_num << " differs:\n";
            std::cout << "    Expected: \"" << expected_line << "\"\n";
            std::cout << "    Actual  : \"" << actual_line << "\"\n";
        }
        ++line_num;
    }

    // Handle extra lines
    while (std::getline(expected_stream, expected_line)) {
        std::cout << "  Line " << line_num << " missing in actual output:\n";
        std::cout << "    Expected: \"" << expected_line << "\"\n";
        ++line_num;
    }

    while (std::getline(actual_stream, actual_line)) {
        std::cout << "  Extra line " << line_num << " in actual output:\n";
        std::cout << "    Actual  : \"" << actual_line << "\"\n";
        ++line_num;
    }
}

std::string adjust_tool_path(const std::string& tool_path, const std::string& exeext) {
    size_t start = tool_path.find("tools/");
    if (start != std::string::npos) {
        size_t space = tool_path.find(' ', start);
        std::string path = (space != std::string::npos) ? tool_path.substr(0, space) : tool_path;
        std::string rest = (space != std::string::npos) ? tool_path.substr(space) : "";

        std::string lt_path = path;
        if (lt_path.find(".exe") == std::string::npos)
            lt_path += exeext;

        std::string libtool_wrapper = "tools/.libs/lt-" + lt_path.substr(6);
        FILE* f = fopen(libtool_wrapper.c_str(), "r");
        if (f) {
            fclose(f);
            return libtool_wrapper + rest;
        }
    }
    return tool_path;
}


int run_test(const std::string& cmd, 
    FILE* expected_stdout, 
    int expected_exit, 
    TestResult& result) 
{
    std::string exeext = std::getenv("EXEEXT") ? std::getenv("EXEEXT") : "";
    std::string data_dir = std::getenv("SLEUTHKIT_TEST_DATA_DIR") ? std::getenv("SLEUTHKIT_TEST_DATA_DIR") : "";

    std::string resolved_cmd = cmd;

    size_t pos;
    while ((pos = resolved_cmd.find("$EXEEXT")) != std::string::npos) {
    resolved_cmd.replace(pos, 7, exeext);
    }
    while ((pos = resolved_cmd.find("$SLEUTHKIT_TEST_DATA_DIR")) != std::string::npos) {
    resolved_cmd.replace(pos, 24, data_dir);
    }

    resolved_cmd = adjust_tool_path(resolved_cmd, exeext);

    // Use named tempfile for command output
    std::string tmpfile_path;
    FILE* tmpfile = tsk_make_named_tempfile(&tmpfile_path);
    if (!tmpfile) {
        std::cerr << "Failed to create temp file for command output.\n";
        result.error = true;
        return 1;
    }
    std::fclose(tmpfile); // Close right away, we'll reopen it after execution

    std::string full_cmd = resolved_cmd + " > \"" + tmpfile_path + "\" 2>&1";
    std::cout << "[exec] " << full_cmd << '\n';

    int exit_code = std::system(full_cmd.c_str());

#if defined(_WIN32) && defined(__MINGW32__)
    result.actual_exit = exit_code;
#else
    result.actual_exit = WEXITSTATUS(exit_code);
#endif

    // Read actual output from temp file
    std::ifstream tmpin(tmpfile_path.c_str());
    std::ostringstream actual_output_stream;
    actual_output_stream << tmpin.rdbuf();
    std::string actual_output = actual_output_stream.str();
    tmpin.close();
    std::remove(tmpfile_path.c_str());

    // Read expected output
    std::string expected_output = read_file(expected_stdout);

    result.stdout_match = (actual_output == expected_output);
    if (!result.stdout_match) {
        std::cout << "  [diff] stdout mismatch in test: " << result.id << "\n";
        print_diff(expected_output, actual_output);
    }

    result.stderr_match = true; // stderr is redirected to stdout
    result.error = (result.actual_exit != expected_exit || !result.stdout_match);

    return result.error ? 1 : 0;
}



// Print result summary table
void print_summary(const std::vector<TestResult>& results) {
    std::cout << "\nTest Summary:\n";
    std::cout << std::setw(12) << "Test ID"
        << std::setw(10) << "Exit"
        << std::setw(10) << "Match"
        << "\n";

    for (const auto& r : results) {
    std::cout << std::setw(12) << r.id
            << std::setw(10) << r.actual_exit
            << std::setw(10) << (r.stdout_match && r.stderr_match ? "yes" : "NO")
            << "\n";

    if (!r.stdout_match)
    std::cout << "  stdout mismatch for test: " << r.id << "\n";
    if (!r.stderr_match)
    std::cout << "  stderr mismatch or unexpected stderr in: " << r.id << "\n";
    }
    if (results.empty()) {
        std::cout << "[!] No tests were run or no results were recorded.\n";
    }
}

// Run all tests from definition file
int run_all_tests() {

    int tests_run = 0, tests_skipped = 0, tests_failed = 0;
    std::vector<TestResult> results;

    std::ifstream infile("test/tools/cli_tests.txt");
    if (!infile) {
        perror("open");
    }


    std::string line;
    while (std::getline(infile, line)) {
        if (line.empty() || line[0] == '#') continue;

        std::string id, cmd, expected_file;
        int expected_exit;
        if (!parse_test_line(line, id, cmd, expected_file, expected_exit)) {
            std::cerr << "Invalid line: " << line << "\n";
            continue;
        }

        TestResult result{id, cmd, expected_exit};
        tests_run++;

        if (cmd.empty()) {
            result.skipped = true;
            tests_skipped++;
        } else {
            FILE* expected = fopen(expected_file.c_str(), "r");
            if (!expected) {
                std::cerr << "Failed to open expected output file: " << expected_file << "\n";
                result.error = true;
            } else {
                run_test(cmd, expected, expected_exit, result);
            }
        if (result.error) tests_failed++;
        }
        results.push_back(result);
    }

    print_summary(results);

    std::cout << "\nTests run: " << tests_run
        << ", Skipped: " << tests_skipped
        << ", Failed: " << tests_failed << "\n";

    return (tests_failed == 0) ? 0 : 1;
}

// Entry point
int main() {
    return run_all_tests();
}
