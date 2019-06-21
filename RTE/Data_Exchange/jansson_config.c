#include <stdint.h>
#include <xmc4500.h>

/* gettimeofday() and getpid() */
int seed_from_timestamp_and_pid(uint32_t *seed) {
    *seed = SysTick->VAL;
    return 0;
}
