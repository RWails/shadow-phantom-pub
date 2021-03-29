/*
 * The Shadow Simulator
 * See LICENSE for licensing information
 */

#include <assert.h>
#include <errno.h>
#include <stdarg.h>
#include <stdbool.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <time.h>

#include "shim/shim.h"
#include "shim/shim_syscall.h"
#include "support/logger/logger.h"

#define SIMTIME_NANOS_PER_SEC 1000000000l
#define SIMTIME_MICROS_PER_NANOSEC 1000l

// We store the simulation time using timespec to reduce the number of
// conversions that we need to do while servicing syscalls.
static struct timespec _cached_simulation_time = {0};

void shim_syscall_set_simtime_nanos(uint64_t simulation_nanos) {
    _cached_simulation_time.tv_sec = simulation_nanos / SIMTIME_NANOS_PER_SEC;
    _cached_simulation_time.tv_nsec = simulation_nanos % SIMTIME_NANOS_PER_SEC;
}

uint64_t shim_syscall_get_simtime_nanos() {
    return (uint64_t)(_cached_simulation_time.tv_sec * SIMTIME_NANOS_PER_SEC) +
           _cached_simulation_time.tv_nsec;
}

bool shim_syscall_is_supported(long syscall_num) {
    return syscall_num == SYS_clock_gettime || syscall_num == SYS_time ||
           syscall_num == SYS_gettimeofday;
}

static struct timespec* _shim_syscall_get_time() {
    // First try to get time from shared mem.
    struct timespec* simtime_ts = shim_get_shared_time_location();

    // If that's unavailable, check if the time has been cached before.
    if (simtime_ts == NULL) {
        simtime_ts = &_cached_simulation_time;
    }

    // If the time is not set, then we fail.
    if (simtime_ts->tv_sec == 0 && simtime_ts->tv_nsec == 0) {
        return NULL;
    }

#ifdef DEBUG
    if (simtime_ts == &_cached_simulation_time) {
        debug("simtime is available in the shim using cached time");
    } else {
        debug("simtime is available in the shim using shared memory");
    }
#endif

    return simtime_ts;
}

bool shim_syscall(long syscall_num, long* rv, va_list args) {
#ifdef DEBUG
    assert(shim_syscall_is_supported(syscall_num));
#endif

    // We currently only support time syscalls, so return if we don't have the time.
    struct timespec* simtime_ts = _shim_syscall_get_time();
    if (!simtime_ts) {
        return false;
    }

    switch (syscall_num) {
        case SYS_clock_gettime: {
            debug("servicing syscall %ld:clock_gettime from the shim", syscall_num);

            clockid_t clk_id = va_arg(args, clockid_t);
            struct timespec* tp = va_arg(args, struct timespec*);

            if (tp) {
                *tp = *simtime_ts;
                debug("clock_gettime() successfully copied time");
                *rv = 0;
            } else {
                debug("found NULL timespec pointer in clock_gettime");
                *rv = -1;
                errno = EFAULT;
            }

            break;
        }

        case SYS_time: {
            debug("servicing syscall %ld:time from the shim", syscall_num);

            time_t* tp = va_arg(args, time_t*);

            if (tp) {
                *tp = simtime_ts->tv_sec;
                debug("time() successfully copied time");
            }
            *rv = simtime_ts->tv_sec;

            break;
        }

        case SYS_gettimeofday: {
            debug("servicing syscall %ld:gettimeofday from the shim", syscall_num);

            struct timeval* tp = va_arg(args, struct timeval*);

            if (tp) {
                tp->tv_sec = simtime_ts->tv_sec;
                tp->tv_usec = simtime_ts->tv_nsec / SIMTIME_MICROS_PER_NANOSEC;
                debug("gettimeofday() successfully copied time");
            }
            *rv = 0;

            break;
        }

        default: {
            debug("syscall %ld is not a supported time-related syscall", syscall_num);
            // the syscall was not handled
            return false;
        }
    }

    // the syscall was handled
    return true;
}