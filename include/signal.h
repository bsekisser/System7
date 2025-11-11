/*
 * signal.h - Minimal signal handling for bare-metal ARM64 kernel
 */

#ifndef _SIGNAL_H_
#define _SIGNAL_H_

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Signal types */
typedef int sig_t;

/* Standard signal numbers */
#define SIGHUP      1
#define SIGINT      2
#define SIGQUIT     3
#define SIGILL      4
#define SIGTRAP     5
#define SIGABRT     6
#define SIGBUS      7
#define SIGFPE      8
#define SIGKILL     9
#define SIGUSR1     10
#define SIGSEGV     11
#define SIGUSR2     12
#define SIGPIPE     13
#define SIGALRM     14
#define SIGTERM     15

/* Minimal signal handling - most are no-ops in bare-metal */
typedef void (*sighandler_t)(int);

sighandler_t signal(int sig, sighandler_t handler);
int raise(int sig);

#ifdef __cplusplus
}
#endif

#endif /* _SIGNAL_H_ */
