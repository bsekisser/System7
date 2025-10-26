#include <signal.h>

int raise(int signum) {
    (void)signum;
    return 0;
}

/* errno stub for static linking - must be TLS for libm compatibility */
__thread int errno = 0;

/* Stack protection stubs */
void __stack_chk_fail(void) {
    /* Stack check failed - hang */
    while (1) {}
}

void __stack_chk_fail_local(void) {
    __stack_chk_fail();
}
