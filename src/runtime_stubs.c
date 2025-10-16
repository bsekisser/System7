#include <signal.h>

int raise(int signum) {
    (void)signum;
    return 0;
}
