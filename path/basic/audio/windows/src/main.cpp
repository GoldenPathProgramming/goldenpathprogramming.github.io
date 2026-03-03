#include <windows.h>
#include <xaudio2.h>
#include <avrt.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <conio.h>

#define SAMPLING_RATE 48000
#define PERIOD_SIZE (SAMPLING_RATE / 200)
#define LOG(format__, ...) printf(format__ "\n" __VA_OPT__(,__VA_ARGS__))

static IXAudio2* xaudio = nullptr;
static IXAudio2MasteringVoice* xaudio_master_voice = nullptr;
static IXAudio2SourceVoice* xaudio_source_voice = nullptr;
static XAUDIO2_BUFFER xaudio_buffers[2];
static float audio_buffers[2][PERIOD_SIZE] = {};
static bool restart_audio = true;

static int frequency = 440;
static int wavelength = SAMPLING_RATE / 440;
static void CalculateWavelength () {
    if (frequency < 40) frequency = 40;
    if (frequency > 10000) frequency = 10000;
    wavelength = SAMPLING_RATE / frequency;
}

const char *HResultToStr (HRESULT result) {
    static char error_buffer[512];
    DWORD len;  // Number of chars returned.
    len = FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, result, 0, error_buffer, 512, NULL);
    if (len == 0) {
        if (len == 0) snprintf (error_buffer, sizeof (error_buffer), "HRESULT error message not found");
    }
    return error_buffer;
}

#define DO_OR_QUIT(function_call__, error_message__) do { auto result = function_call__; assert (result == S_OK); if (result != S_OK) { LOG (error_message__ " [%lx] [%s]", result, HResultToStr(result)); return 1; } } while (0)
#define DO_OR_RESTART(function_call__, error_message__) { auto result = function_call__; if (result != S_OK) { LOG (error_message__ " [%lx] [%s]", result, HResultToStr(result)); restart_audio = true; continue; } }

class VoiceCallback : public IXAudio2VoiceCallback {
public:
    void OnBufferEnd(void* pBufferContext) override {
        static bool sample_buffer_swap = 0;
        float *data = audio_buffers[sample_buffer_swap];
        for (int i = 0; i < PERIOD_SIZE; ++i) {
            static uint16_t sound_counter = 0;
            float sample = sound_counter < wavelength/2 ? 1 : -1;
            if (++sound_counter >= wavelength) sound_counter = 0;
            *data++ = sample;
        }
        xaudio_source_voice->SubmitSourceBuffer (&xaudio_buffers[sample_buffer_swap], nullptr);
        sample_buffer_swap = !sample_buffer_swap;;
    }

    void OnStreamEnd() {}
    void OnVoiceProcessingPassEnd() {}
    void OnVoiceProcessingPassStart(UINT32 SamplesRequired) {}
    void OnBufferStart(void* pBufferContext) {}
    void OnLoopEnd(void* pBufferContext) {}
    void OnVoiceError(void* pBufferContext, HRESULT Error) {
        restart_audio = true;
        LOG ("Critical XAudio2 voice error [%lx] [%s]", Error, HResultToStr(Error));
    }
};

class EngineCallback : public IXAudio2EngineCallback {
public:
    void OnCriticalError(HRESULT Error) override {
        restart_audio = true;
        LOG ("Critical XAudio2 Engine error [%lx] [%s]", Error, HResultToStr(Error));
    }
    void OnProcessingPassEnd() {}
    void OnProcessingPassStart() {}
};

VoiceCallback xaudio_voice_callback;
EngineCallback xaudio_engine_callback;

const WAVEFORMATEX wave_format = {
    .wFormatTag = WAVE_FORMAT_IEEE_FLOAT,
    .nChannels = 1,
    .nSamplesPerSec = SAMPLING_RATE,
    .nAvgBytesPerSec = SAMPLING_RATE * sizeof (float),
    .nBlockAlign = sizeof (float),
    .wBitsPerSample = sizeof (float) * 8,
    .cbSize = 0
};

typedef HRESULT (WINAPI *XAudio2Create_t)(IXAudio2**, UINT32, XAUDIO2_PROCESSOR);

int main () {
    DO_OR_QUIT (CoInitializeEx(NULL, COINIT_MULTITHREADED), "Sound thread failed to CoInitializeEx");

    { DWORD task_index = 0; AvSetMmThreadCharacteristics(TEXT("Pro Audio"), &task_index); }

    HMODULE xaudio_dll = nullptr;
    bool multiple_restarts = false;
    bool quit = false;
    while (restart_audio && !quit) {
        LOG ("Initializing xaudio");
        if (multiple_restarts) Sleep (1000);
        multiple_restarts = true;

        if (xaudio_source_voice != nullptr) { xaudio_source_voice->DestroyVoice(); xaudio_source_voice = nullptr; }
        if (xaudio_master_voice != nullptr) { xaudio_master_voice->DestroyVoice(); xaudio_master_voice = nullptr; }
        if (xaudio != nullptr) { xaudio->Release(); xaudio = nullptr; }
        if (xaudio_dll) { FreeLibrary (xaudio_dll); xaudio_dll = nullptr; }

        xaudio_dll = LoadLibraryA("xaudio2_9.dll");
        if (!xaudio_dll) { LOG("Failed to load xaudio2_9.dll"); restart_audio = true; continue; }
        XAudio2Create_t XAudio2Create = (XAudio2Create_t)GetProcAddress (xaudio_dll, "XAudio2Create"); assert (XAudio2Create);
        if (!XAudio2Create) { LOG ("Failed to get XAudio2Create"); restart_audio = true; continue; }
        // You can delete the above lines if you create the lib file with the following commands (run in the same directory as main.cpp):
        // gendef C:\Windows\System32\xaudio2_9.dll
        // dlltool -d xaudio2_9.def -l libxaudio2.a
        // Also add "-L. -lxaudio2" to your compilation command

        DO_OR_RESTART (XAudio2Create(&xaudio, 0, XAUDIO2_DEFAULT_PROCESSOR), "Sound thread failed to create XAudio2");

        DO_OR_RESTART (xaudio->RegisterForCallbacks(&xaudio_engine_callback), "Failed to register for engine callbacks");

        DO_OR_RESTART (xaudio->CreateMasteringVoice(&xaudio_master_voice, 1, SAMPLING_RATE, 0, NULL, NULL, AudioCategory_GameEffects), "Failed to create mastering voice");

        DO_OR_RESTART (xaudio->CreateSourceVoice(&xaudio_source_voice, &wave_format, 0, XAUDIO2_DEFAULT_FREQ_RATIO, &xaudio_voice_callback, NULL, NULL), "Failed to create source voice");

        DO_OR_RESTART (xaudio_source_voice->Start(0, XAUDIO2_COMMIT_NOW), "Failed to start source voice");
        
        multiple_restarts = restart_audio = false;

        memset (audio_buffers, 0, sizeof (audio_buffers));
        xaudio_buffers[0].AudioBytes = xaudio_buffers[1].AudioBytes = sizeof(float) * PERIOD_SIZE;
        xaudio_buffers[0].pAudioData = (const BYTE*)&audio_buffers[0];
        xaudio_buffers[1].pAudioData = (const BYTE*)&audio_buffers[1];

        xaudio_source_voice->SubmitSourceBuffer (&xaudio_buffers[0], nullptr);
        xaudio_source_voice->SubmitSourceBuffer (&xaudio_buffers[1], nullptr);
        
        LOG ("XAudio starting keepalive loop");
        while (!quit && !restart_audio) {
            if (_kbhit ()) {
                unsigned char c = _getch ();
                if (c == 224) {
                    c = _getch ();
                    switch (c) {
                        case 72: frequency += 50; CalculateWavelength(); break; /* Up arrow */
                        case 80: frequency -= 50; CalculateWavelength(); break; /* Down arrow  */
                        default: break;
                    }
                }
                else {
                    switch (c) {
                        case 'q': case 'Q': case 'K': case 'k': case 27: quit = true; break;
                        default: break;
                    }
                }
            }
        }
        LOG ("Xaudio keepalive %s", quit ? "quitting" : (restart_audio ? "restarting" : "!unknown!"));
    }

    CoUninitialize();

    return 0;
}