#ifndef _TSK_PRINTF_H
#define _TSK_PRINTF_H

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Global output streams */
extern FILE *g_tsk_printf_fd;
extern FILE *g_tsk_stderr_fd;

/* Redirection setters */
FILE *tsk_set_printf_fd(FILE *fd);
FILE *tsk_set_stderr_fd(FILE *fd);

/* Printing functions */
void tsk_fprintf(FILE *fd, const char *msg, ...);
void tsk_printf(const char *msg, ...);
int tsk_print_sanitized(FILE *fd, const char *str);

#ifdef __cplusplus
}
#endif

#endif /* _TSK_PRINTF_H */
