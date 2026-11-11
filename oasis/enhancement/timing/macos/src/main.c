#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>

int64_t Timeus () {
    const uint64_t tnsec = clock_gettime_nsec_np (CLOCK_UPTIME_RAW);
    return tnsec / 1000ull;
}

int main () {
    int64_t time_last = Timeus();

    for (int i = 0; i < 5; ++i) {
        sleep (1);
        int64_t time_now = Timeus();
        printf ("Slept 1 second. Measured: %.6f\n", (time_now - time_last) / 1000000.f);
        time_last = time_now;
    }

    return 0;
}
