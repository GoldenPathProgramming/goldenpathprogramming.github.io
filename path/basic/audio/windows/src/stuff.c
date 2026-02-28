#include <stdint.h>
#include <assert.h>
#include <stdio.h>
#include <conio.h>

static bool restart_audio = true;
static bool quit = false;
int32_t sampling_rate = 48000;

#define LOG(...) do { printf(__VA_ARGS__); puts(""); } while (0)

const char *HResultToStr (HRESULT result) {
    static char wasapi_error_buffer[512];
    DWORD len;  // Number of chars returned.
    len = FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, result, 0, wasapi_error_buffer, 512, NULL);
    if (len == 0) {
        HINSTANCE hInst = LoadLibraryA("Ntdsbmsg.dll");
        if (hInst == NULL)
            snprintf (wasapi_error_buffer, sizeof (wasapi_error_buffer), "Cannot convert error to string: Failed to load Ntdsbmsg.dll");
        else {
            len = FormatMessageA(FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_IGNORE_INSERTS, hInst, result, 0, wasapi_error_buffer, 512, NULL);
            if (len == 0) snprintf (wasapi_error_buffer, sizeof (wasapi_error_buffer), "HRESULT error message not found");
            FreeLibrary( hInst );
        }
    }
    return wasapi_error_buffer;
}

static int frequency = 440;
static int wavelength = 48000 / 440;
static void CalculateWavelength () {
    if (frequency < 40) frequency = 40;
    if (frequency > 10000) frequency = 10000;
    wavelength = sampling_rate / frequency;
	LOG ("Wavelength: %dHz", frequency);
}

void SoundLoop_int (WAVEFORMATEXTENSIBLE wave_format, HANDLE event_handle, IAudioRenderClient *render, UINT32 frames_per_period);
void SoundLoop_float (WAVEFORMATEXTENSIBLE wave_format, HANDLE event_handle, IAudioRenderClient *render, UINT32 frames_per_period);

#define DO_OR_QUIT(function_call__, error_message__) do { auto result = function_call__; assert (result == S_OK); if (result != S_OK) { LOG (error_message__ " [%ld] [%s]", result, HResultToStr(result)); return -1; } } while (0)
#define DO_OR_CONTINUE(function_call__, error_message__) do { auto result = function_call__; if (result != S_OK) { LOG (error_message__ " [%ld] [%s]", result, HResultToStr(result)); } } while (0)
#define QUIT_IF_NULL(variable__, error_message__) do { auto __var__ = (variable__); assert (__var__); if (__var__ == NULL) { LOG (error_message__); return -1; } } while (0)
#define IF_NULL_RESTART(variable__, message__) if (variable__ == NULL) { LOG (message__); restart_audio = true; continue; }
#define BREAK_ON_FAIL(code__, message__) { auto result = code__; assert (!FAILED(result)); if (FAILED(result)) { LOG (message__); break; } }
#define DO_OR_RESTART(function_call__, error_message__) ({ auto result = function_call__; if (result != S_OK) { LOG (error_message__ " [%ld] [%s]", result, HResultToStr(result)); restart_audio = true; continue; } })

// Defer implementation by Jens Gustedt: https://gustedt.wordpress.com/2026/02/15/defer-available-in-gcc-and-clang/
#if __has_include(<stddefer.h>)
# include <stddefer.h>
# if defined(__clang__)
#  if __is_identifier(_Defer)
#   error "clang may need the option -fdefer-ts for the _Defer feature"
#  endif
# endif
#elif __GNUC__ > 8
# define defer _Defer
# define _Defer      _Defer_A(__COUNTER__)
# define _Defer_A(N) _Defer_B(N)
# define _Defer_B(N) _Defer_C(_Defer_func_ ## N, _Defer_var_ ## N)
# define _Defer_C(F, V)                                                 \
  auto void F(int*);                                                    \
  __attribute__((__cleanup__(F), __deprecated__, __unused__))           \
     int V;                                                             \
  __attribute__((__always_inline__, __deprecated__, __unused__))        \
    inline auto void F(__attribute__((__unused__)) int*V)
#else
# error "The _Defer feature seems not available"
#endif