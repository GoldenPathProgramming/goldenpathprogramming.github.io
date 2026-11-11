#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>

int64_t Timeus () {
    struct timespec t;
    if (clock_gettime (CLOCK_MONOTONIC, &t) != 0) {
        return 0;
    }
    int64_t ticks = t.tv_sec * 1000000000ll + t.tv_nsec;
    printf ("%ld.%ld\n", t.tv_sec, t.tv_nsec);
    return ticks / 1000;
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
