#if !defined (_RTEMS_RTL_SHELL_H_)
#define _RTEMS_RTL_SHELL_H_

#include <rtems/print.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * The PRUSS single shell command contains sub-commands.
 *
 * @param argc The argument count.
 * @param argv Array of argument strings.
 * @retval 0 No error.
 * @return int The exit code.
 */
int rtems_pruss_shell_command (int argc, char* argv[]);

int rtems_pruss_shell_status (const rtems_printer* printer,
                        int                  argc,
                        char*                argv[]);

int
rtems_pruss_shell_init (const rtems_printer* printer,
                        int                  argc,
                        char*                argv[]);

int
rtems_pruss_shell_alloc (const rtems_printer* printer,
                        int                  argc,
                        char*                argv[]);

int
rtems_pruss_shell_reset (const rtems_printer* printer,
                        int                  argc,
                        char*                argv[]);

int
rtems_pruss_shell_upload (const rtems_printer* printer,
                        int                  argc,
                        char*                argv[]);

int
rtems_pruss_shell_start (const rtems_printer* printer,
                        int                  argc,
                        char*                argv[]);

int
rtems_pruss_shell_step (const rtems_printer* printer,
                        int                  argc,
                        char*                argv[]);

int
rtems_pruss_shell_stop (const rtems_printer* printer,
                        int                  argc,
                        char*                argv[]);

int
rtems_pruss_shell_wait (const rtems_printer* printer,
                        int                  argc,
                        char*                argv[]);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
