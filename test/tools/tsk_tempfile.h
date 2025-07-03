#ifndef _TSK_TEMPFILE_H
#define _TSK_TEMPFILE_H
#include <stdio.h>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <string>
#include <stdio.h>
// Creates a temporary file for use in testing. 
FILE* tsk_make_tempfile();
FILE* tsk_make_named_tempfile(std::string* out_path);

#endif