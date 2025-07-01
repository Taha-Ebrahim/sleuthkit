#ifndef TSK_CLI_RUNNER_H
#define TSK_CLI_RUNNER_H

#include <string>
#include <vector>
#include <cstdio>

// Struct to store result of a single test case
struct TestResult {
    std::string id;
    std::string cmd;
    int expected_exit;
    int actual_exit;
    bool stdout_match;
    bool stderr_match;
    bool skipped;
    bool error;
};

// Parses a single line from the test file.
// Returns true if the line is well-formed, false otherwise.
bool parse_test_line(const std::string& line,
                     std::string& id,
                     std::string& cmd,
                     std::string& expected_file,
                     int& expected_exit);

// Reads and returns the contents of the given FILE*.
std::string read_file(FILE* file);

// Compares the contents of two FILE* streams.
bool compare_files(FILE* expected, FILE* actual);

// Runs a single test case and populates the result object.
// Returns 1 if the test fails, 0 otherwise.
int run_test(const std::string& id,
             const std::string& cmd,
             FILE* expected_stdout,
             int expected_exit,
             TestResult& result,
             std::string tmpdir);

// Prints a summary of all test results to stdout.
void print_summary(const std::vector<TestResult>& results);

// Loads and runs all tests from the cli_tests.txt file.
// Returns 0 on success (all tests passed), 1 otherwise.
int run_all_tests(std::string tmpdir);

#endif // TSK_CLI_RUNNER_H