// Requires clang 22+. Compile with flag -fdefer-ts
#include <AudioToolbox/AudioToolbox.h>
#include <stddefer.h>
#include <unistd.h>
#include <termios.h>

#define SAMPLING_RATE 48000
#define PERIOD_SIZE (SAMPLING_RATE / 200)
#define LOG(format__, ...) printf(format__ __VA_OPT__(,__VA_ARGS__))

#define DO_OR_RETRY(__function__, __error_message__) { OSErr err = __function__; assert (err == 0); if (err != 0) { LOG (__error_message__ "[%d]", err); continue; } }
#define DO_OR_LOG(__function__, __error_message__) { OSErr err = __function__; if (err != 0) { LOG (__error_message__ "[%d]", err); } }

static bool restart_audio = true;

static int frequency = 440;
static int wavelength = SAMPLING_RATE / 440;
static void CalculateWavelength () {
    if (frequency < 40) frequency = 40;
    if (frequency > 10000) frequency = 10000;
    wavelength = SAMPLING_RATE / frequency;
}

static OSStatus SoundCallback (void *inRefCon, AudioUnitRenderActionFlags *ioActionFlags, const AudioTimeStamp *inTimeStamp, UInt32 inBusNumber, UInt32 inNumberFrames, AudioBufferList *ioData) {
    assert (ioData->mNumberBuffers == 1);
    SInt16 *data = (SInt16*)ioData->mBuffers[0].mData;
    for (UInt32 frame = 0; frame < inNumberFrames; ++frame) {
        static uint16_t sound_counter = 0;
        *(data++) = sound_counter < wavelength/2 ? INT16_MIN : INT16_MAX;
        if (++sound_counter >= wavelength) sound_counter = 0;
    }
    return noErr;
}

OSStatus deviceChangedCallback(AudioObjectID inObjectID, UInt32 inNumberAddresses, const AudioObjectPropertyAddress inAddresses[], void* inClientData) {
    LOG ("Default output device changed.\n");
    restart_audio = true;
    return noErr;
}

int main () {
    struct termios settings_new, settings_old;
    tcgetattr (0, &settings_old);
    settings_new = settings_old;
    settings_new.c_lflag &= ~ICANON;
    settings_new.c_lflag &= ~ECHO;
    settings_new.c_lflag &= ~ISIG;
    settings_new.c_cc[VMIN] = 0;
    settings_new.c_cc[VTIME] = 0;
    tcsetattr (0, TCSANOW, &settings_new);
    defer { tcsetattr (0, TCSANOW, &settings_old); }

    bool restart_multiple_attempts = false;
    while (restart_audio && !quit) {
        if (restart_multiple_attempts > 0) sleep (1);
        restart_multiple_attempts = true;

        AudioComponent output;
        { output = AudioComponentFindNext(NULL,
            &(AudioComponentDescription){
                .componentType = kAudioUnitType_Output,
                .componentSubType = kAudioUnitSubType_DefaultOutput,
        }); assert (output); if (!output) { LOG("Can't find default output"); continue; } }

        AudioUnit tone_unit;
        DO_OR_RETRY (AudioComponentInstanceNew(output, &tone_unit), "Error creating audio unit");
        defer { DO_OR_LOG (AudioComponentInstanceDispose(tone_unit), "Error disposing audio unit"); }

        DO_OR_RETRY (AudioUnitSetProperty(tone_unit, kAudioUnitProperty_SetRenderCallback, kAudioUnitScope_Input, 0, &(AURenderCallbackStruct){.inputProc = SoundCallback}, sizeof(AURenderCallbackStruct)), "Error setting render callback");

        AudioStreamBasicDescription asbd = {
            .mFormatID = kAudioFormatLinearPCM,
            .mFormatFlags = kAudioFormatFlagIsSignedInteger | kAudioFormatFlagIsPacked | kAudioFormatFlagIsNonInterleaved,
            .mSampleRate = SAMPLING_RATE,
            .mBitsPerChannel = 16,
            .mChannelsPerFrame = 1,
            .mFramesPerPacket = 1,
            .mBytesPerFrame = 2,
            .mBytesPerPacket = 2,
        };
        DO_OR_RETRY (AudioUnitSetProperty(tone_unit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Input, 0, &asbd, sizeof(asbd)), "Error setting stream format");

        DO_OR_LOG (AudioUnitSetProperty(tone_unit, kAudioDevicePropertyBufferFrameSize, kAudioUnitScope_Input, 0, &(UInt32){PERIOD_SIZE}, sizeof(UInt32)), "Error setting buffer size");

        AudioObjectPropertyAddress property_address = {
            .mSelector = kAudioHardwarePropertyDefaultOutputDevice,
            .mScope = kAudioObjectPropertyScopeGlobal,
            .mElement = kAudioObjectPropertyElementMain
        };
        DO_OR_LOG (AudioObjectAddPropertyListener (kAudioObjectSystemObject, &property_address, deviceChangedCallback, NULL), "Failed to add property listener for default device change");
        defer { DO_OR_LOG (AudioObjectRemovePropertyListener (kAudioObjectSystemObject, &property_address, deviceChangedCallback, NULL), "Failed to remove property listener for default device change"); }

        DO_OR_RETRY (AudioUnitInitialize(tone_unit), "Error initializing unit");
        defer { DO_OR_LOG (AudioUnitUninitialize(tone_unit), "Error uninitializing unit"); }

        DO_OR_RETRY (AudioOutputUnitStart(tone_unit), "Error starting unit");
        defer { DO_OR_LOG (AudioOutputUnitStop(tone_unit), "Error stopping unit"); }

        restart_audio = restart_multiple_attempts = false;
        
        bool quit = false;
        while (!quit && !restart_audio) {
            UInt32 is_running = 0;
            if (AudioUnitGetProperty(tone_unit, kAudioOutputUnitProperty_IsRunning, kAudioUnitScope_Global, 0, &is_running, &(UInt32){sizeof(is_running)}) != noErr || !is_running) {
                LOG ("Device not running");
                restart_audio = true;
                break;
            }

            char c;
            if (read (STDIN_FILENO, &c, 1)) {
                if (c == 27) read (STDIN_FILENO, &c, 1); // Escape sequence starting character is 27
                switch (c) {
                    case 65: frequency += 50; CalculateWavelength(); break;
                    case 66: frequency -= 50; CalculateWavelength(); break;
                    case 'q': case 'Q': case 'K': case 'k': case 27: quit = true; break;
                }
            }
        }
    }

    return 0;
}