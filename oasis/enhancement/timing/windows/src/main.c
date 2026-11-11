#include <windows.h>
#include <stdio.h>
#include <stdint.h>

int64_t Timeus () {
    static int64_t ticks_per_microsecond;
	if (ticks_per_microsecond == 0) {
		LARGE_INTEGER frequency;
		QueryPerformanceFrequency (&frequency);
		ticks_per_microsecond = frequency.QuadPart / 1000000ll;
	}

	LARGE_INTEGER ticks;
	QueryPerformanceCounter (&ticks);
	return ticks.QuadPart / ticks_per_microsecond;
}

int main () {
    int64_t time_last = Timeus();

    for (int i = 0; i < 5; ++i) {
        Sleep (1000);
        int64_t time_now = Timeus();
        printf ("Slept 1 second. Measured: %.6f\n", (time_now - time_last) / 1000000.f);
        time_last = time_now;
    }

    return 0;
}
