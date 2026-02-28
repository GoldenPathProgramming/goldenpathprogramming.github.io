#include <pulse/simple.h>
#include <pulse/error.h>
#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <termios.h>

#define SAMPLING_RATE 48000
#define PERIOD_SIZE (SAMPLING_RATE / 200)

static int frequency = 440;
static int wavelength = SAMPLING_RATE / 440;
static void CalculateWavelength () {
    if (frequency < 40) frequency = 40;
    if (frequency > 10000) frequency = 10000;
    wavelength = SAMPLING_RATE / frequency;
}

int main() {
    int pulse_error = 0;
    pa_simple *simple;
    pa_sample_spec sample_spec = {
        .format = PA_SAMPLE_S16LE,
        .rate = SAMPLING_RATE,
        .channels = 1
    };
    pa_buffer_attr buffer_attr = {
        .maxlength = PERIOD_SIZE * 16,
        .tlength = PERIOD_SIZE * 2,
        .prebuf = PERIOD_SIZE * 2,
        .minreq = UINT32_MAX,
        .fragsize = PERIOD_SIZE * 2,
    };
    simple = pa_simple_new (NULL, "Golden Path", PA_STREAM_PLAYBACK, NULL, "playback", &sample_spec, NULL, &buffer_attr, &pulse_error);
    if (!simple) {
        printf ("Failed to initialize PulseAudio: %s\n", pa_strerror(pulse_error));
        assert (false);
    }

    struct termios settings_new, settings_old;
    tcgetattr (0, &settings_old);
    settings_new = settings_old;
    settings_new.c_lflag &= ~ICANON;
    settings_new.c_lflag &= ~ECHO;
    settings_new.c_lflag &= ~ISIG;
    settings_new.c_cc[VMIN] = 0;
    settings_new.c_cc[VTIME] = 0;
    tcsetattr (0, TCSANOW, &settings_new);

    int16_t buffer[PERIOD_SIZE];
    bool quit = false;
    uint16_t sound_counter = 0;
    while (!quit) {
        char c;
        if (read (STDIN_FILENO, &c, 1)) {
            if (c == 27) read (STDIN_FILENO, &c, 1); // Escape sequence starting character is 27
            switch (c) {
                case 65: frequency += 50; CalculateWavelength(); break;
                case 66: frequency -= 50; CalculateWavelength(); break;
                case 'q': case 'Q': case 'K': case 'k': case 27: quit = true; break;
            }
        }

        for (int i = 0; i < PERIOD_SIZE; ++i) {
            buffer[i] = sound_counter < wavelength/2 ? INT16_MIN : INT16_MAX;
            if (++sound_counter >= wavelength) sound_counter = 0;
        }

        const auto result = pa_simple_write (simple, buffer, sizeof(buffer), &pulse_error);
        if (result != 0) {
            printf ("Failed to write buffer: %s\n", pa_strerror (pulse_error));
            quit = true;
        }
    }

    tcsetattr (0, TCSANOW, &settings_old);
    return 0;
}