/* 
Creates a temporary file path for use in testing. It is primarily used on MinGW systems, 
where std::tmpfile() is unreliable 

See test/tsk/img/test_img_types.cpp for example usage. 
*/

#include "tsk_tempfile.h"

#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <string>
#include <stdio.h>

#ifdef _WIN32
#include <windows.h>
#define PATH_SEP '\\'
#else
#include <unistd.h>
#define PATH_SEP '/'
#endif

FILE* tsk_make_tempfile() {
#if defined(_WIN32) && defined(__MINGW32__)
    // MinGW-specific fallback — generate a unique filename and open manually
    std::string temp_dir;
    const char* env = std::getenv("TEMP");
    if (!env) env = std::getenv("TMP");
    temp_dir = env ? env : ".";

    std::string filename = "tsk_tempfile_" + std::to_string(std::time(nullptr)) +
                            "_" + std::to_string(GetTickCount64()) + ".txt";
    std::string full_path = temp_dir + PATH_SEP + filename;

    FILE* file = std::fopen(full_path.c_str(), "w+");
    return file;
#else
    // Use standard std::tmpfile for non-MinGW systems
    return std::tmpfile();
#endif
}

FILE* tsk_make_named_tempfile(std::string* out_path) {
    if (!out_path) return nullptr;

#if defined(_WIN32) && defined(__MINGW32__)
    const char* env = std::getenv("TEMP");
    if (!env) env = std::getenv("TMP");
    std::string temp_dir = env ? env : ".";

    std::string filename = "tsk_tempfile_" +
        std::to_string(std::time(nullptr)) + "_" +
        std::to_string(GetTickCount64()) + ".txt";

    std::string full_path = temp_dir + PATH_SEP + filename;
    FILE* file = std::fopen(full_path.c_str(), "w+");
    if (file) *out_path = full_path;
    return file;

#else
    char tmpl[] = "/tmp/tsk_tempfile_XXXXXX";
    int fd = mkstemp(tmpl);
    if (fd == -1) return nullptr;

    FILE* file = fdopen(fd, "w+");
    if (file) *out_path = tmpl;
    else close(fd); // avoid leaking fd on failure
    return file;
#endif
}
