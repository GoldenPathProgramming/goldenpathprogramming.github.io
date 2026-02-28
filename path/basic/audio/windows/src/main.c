// Build: gcc main.c -lOle32 -lAvrt -lksuser

// WASPI includes
#include <Objbase.h> // CoInitializeEx
#include <initguid.h> // IID stuff
#include <mmdeviceapi.h> // IMMDevice etc
#include <audioclient.h> // IAudioClient3 etc
#include <avrt.h> // AvSetMmThreadCharacteristics
#include <Functiondiscoverykeys_devpkey.h> // PKEY_Device_FriendlyName

#include "stuff.c"

// MMNoitifcationClient stuff to auto-switch when default device changes

static STDMETHODIMP MMNotificationClient_QueryInterface(IMMNotificationClient *self, REFIID riid, void **ppvObject) {
    if (IsEqualIID(riid, &IID_IUnknown) || IsEqualIID(riid, &IID_IMMNotificationClient)) {
        *ppvObject = self;
        return S_OK;
    }
    else {
       *ppvObject = NULL;
        return E_NOINTERFACE;
    }
}

static ULONG MMNotificationClient_AddRef(IMMNotificationClient *self) { return 1; } // Just don't count the references
static ULONG MMNotificationClient_Release(IMMNotificationClient *self) { return 0; }
static STDMETHODIMP MMNotificationClient_OnDeviceStateChanged(IMMNotificationClient *self, LPCWSTR wid, DWORD state) { LOG ("WASAPI device state changed"); return S_OK; }
static STDMETHODIMP MMNotificationClient_OnDeviceAdded(IMMNotificationClient *self, LPCWSTR wid) { LOG ("WASAPI device added"); return S_OK; }
static STDMETHODIMP MMNotificationClient_OnDeviceRemoved(IMMNotificationClient *self, LPCWSTR wid) { LOG ("WASAPI device removed"); return S_OK; }
static STDMETHODIMP MMNotificationClient_OnPropertyValueChanged(IMMNotificationClient *self, LPCWSTR wid, const PROPERTYKEY key) { LOG ("WASAPI property value changed"); return S_OK; }

static STDMETHODIMP MMNotificationClient_OnDefaultDeviceChange(IMMNotificationClient *self, EDataFlow flow, ERole role, LPCWSTR wid) {
    LOG ("WASAPI default device changed");
    restart_audio = true;
    return S_OK;
}

static IMMNotificationClient notification_client = {
    .lpVtbl = &(struct IMMNotificationClientVtbl){
        MMNotificationClient_QueryInterface,
        MMNotificationClient_AddRef,
        MMNotificationClient_Release,
        MMNotificationClient_OnDeviceStateChanged,
        MMNotificationClient_OnDeviceAdded,
        MMNotificationClient_OnDeviceRemoved,
        MMNotificationClient_OnDefaultDeviceChange,
        MMNotificationClient_OnPropertyValueChanged,
    }
};

int main () {
    DO_OR_QUIT (CoInitializeEx(NULL, 0), "Failed to CoInitializeEx");
    defer {CoUninitialize();}

	{ DWORD task_index = 0; if (AvSetMmThreadCharacteristics(TEXT("Pro Audio"), &task_index) == 0) { LOG ("WASAPI failed to set thread characteristics to \"Pro Audio\""); } }

	IMMDeviceEnumerator *device_enumerator = NULL;
	DO_OR_QUIT (CoCreateInstance(&CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL, &IID_IMMDeviceEnumerator, (void**)&device_enumerator), "WASAPI failed to create Device Enumerator");
    QUIT_IF_NULL (device_enumerator, "WASAPI failed to create Device Enumerator, even though CoCreateInstance returned S_OK");
	defer {device_enumerator->lpVtbl->Release(device_enumerator);}

    while (restart_audio) {
        restart_audio = false;
        LOG ("Initializing WASAPI device");

        device_enumerator->lpVtbl->RegisterEndpointNotificationCallback (device_enumerator, &notification_client);
        defer {device_enumerator->lpVtbl->UnregisterEndpointNotificationCallback (device_enumerator, &notification_client);}

        IMMDevice *default_device = NULL;
        DO_OR_RESTART (device_enumerator->lpVtbl->GetDefaultAudioEndpoint(device_enumerator, eRender, eConsole, &default_device), "WASAPI failed to get default audio endpoint");
        IF_NULL_RESTART (default_device, "WASAPI failed to create get default audio endpoint, even though GetDefaultAudioEndpoint returned S_OK");

        do { // Print default audio device friendly name
            LPWSTR id = NULL;
            BREAK_ON_FAIL (default_device->lpVtbl->GetId (default_device, &id), "Couldn't get default device ID"); 

            IPropertyStore *properties = NULL;
            BREAK_ON_FAIL (default_device->lpVtbl->OpenPropertyStore (default_device, STGM_READ, &properties), "Couldn't open property store of default device");
            defer {properties->lpVtbl->Release (properties);}

            PROPVARIANT property_name;
            PropVariantInit (&property_name);
            BREAK_ON_FAIL (properties->lpVtbl->GetValue (properties, &PKEY_Device_FriendlyName, &property_name), "Failed to get default audio device name");
            defer {PropVariantClear (&property_name);}

            if (property_name.vt != VT_EMPTY) LOG ("Default audio device selected: [%S]", property_name.pwszVal);
            else LOG ("Default audio device name blank?");
        } while (0);

        IAudioClient3 *client = NULL;
        DO_OR_RESTART (default_device->lpVtbl->Activate(default_device, &IID_IAudioClient3, CLSCTX_ALL, NULL, (void**)&client), "WASAPI failed to activate audio client");
        IF_NULL_RESTART (client, "WASAPI failed to activate audio client, even though Activate returned S_OK");
        defer {client->lpVtbl->Release(client);}

        default_device->lpVtbl->Release(default_device);

        AudioClientProperties audio_properties = {
            .cbSize = sizeof(AudioClientProperties),
            .bIsOffload = FALSE,
            .eCategory = AudioCategory_GameEffects,
        };
        DO_OR_RESTART (client->lpVtbl->SetClientProperties(client, &audio_properties), "WASAPI failed to set client properties");

        WAVEFORMATEXTENSIBLE wave_format = {
            .Format = {
                .wFormatTag = WAVE_FORMAT_PCM,
                .nChannels = 2,
                .nSamplesPerSec = sampling_rate,
                .wBitsPerSample = 16,
            },
        };
        wave_format.Format.nBlockAlign = wave_format.Format.nChannels * wave_format.Format.wBitsPerSample / 8;
        wave_format.Format.nAvgBytesPerSec = wave_format.Format.nSamplesPerSec * wave_format.Format.nBlockAlign;

        {
            WAVEFORMATEX *wave_format_closest;
            HRESULT result = client->lpVtbl->IsFormatSupported (client, AUDCLNT_SHAREMODE_SHARED, &wave_format.Format, &wave_format_closest);
            if (result == S_OK) {
                LOG ("Default audio format supported");
            }
            else if (result == S_FALSE && wave_format_closest != NULL) {
                wave_format = *(WAVEFORMATEXTENSIBLE*)wave_format_closest;
                LOG ("Default audio format unsupported. Using this instead: Channels[%d] Bits per sample[%d] Sampling rate[%d] Extensible[%s] Valid bits per sample[%d] Format: [", wave_format.Format.nChannels, wave_format.Format.wBitsPerSample, wave_format.Format.nSamplesPerSec, wave_format_closest->wFormatTag == WAVE_FORMAT_EXTENSIBLE ? "true" : "false", wave_format.Samples.wValidBitsPerSample);
                sampling_rate = wave_format.Format.nSamplesPerSec;
                if (IsEqualGUID(&wave_format.SubFormat, &KSDATAFORMAT_SUBTYPE_PCM)) LOG ("KSDATAFORMAT_SUBTYPE_PCM]");
                else if (IsEqualGUID(&wave_format.SubFormat, &KSDATAFORMAT_SUBTYPE_IEEE_FLOAT)) LOG ("KSDATAFORMAT_SUBTYPE_IEEE_FLOAT]");
                else if (IsEqualGUID(&wave_format.SubFormat, &KSDATAFORMAT_SUBTYPE_DRM)) LOG ("KSDATAFORMAT_SUBTYPE_DRM]");
                else if (IsEqualGUID(&wave_format.SubFormat, &KSDATAFORMAT_SUBTYPE_ALAW)) LOG ("KSDATAFORMAT_SUBTYPE_ALAW]");
                else if (IsEqualGUID(&wave_format.SubFormat, &KSDATAFORMAT_SUBTYPE_MULAW)) LOG ("KSDATAFORMAT_SUBTYPE_MULAW]");
                else if (IsEqualGUID(&wave_format.SubFormat, &KSDATAFORMAT_SUBTYPE_ADPCM)) LOG ("KSDATAFORMAT_SUBTYPE_ADPCM]");
                else { LOG ("UNKNOWN]"); return -1; }
            }
            else { LOG ("No audio format supported"); return -1; }
        }

        uint32_t period_default = 0, period_fundamental = 0, period_min = 0, period_max = 0;
        DO_OR_RESTART (client->lpVtbl->GetSharedModeEnginePeriod (client, &wave_format.Format, &period_default, &period_fundamental, &period_min, &period_max), "WASAPI failed to get shared mode engine period");
        uint32_t frames_per_period = period_min;
        LOG ("Device min period [%u] requested period [%u] samples [%.2fms]", period_min, frames_per_period, 1000.f * frames_per_period / wave_format.Format.nSamplesPerSec);
        DO_OR_RESTART (client->lpVtbl->InitializeSharedAudioStream(client, AUDCLNT_STREAMFLAGS_EVENTCALLBACK, frames_per_period, &wave_format.Format, NULL), "WASAPI failed to initialize shared audio stream");

        HANDLE event_handle = CreateEvent (NULL, FALSE, FALSE, NULL);
        IF_NULL_RESTART (event_handle, "Failed to create event handle");
        DO_OR_RESTART (client->lpVtbl->SetEventHandle (client, event_handle), "WASAPI failed to set event handle");

        IAudioRenderClient *render = NULL;
        DO_OR_RESTART (client->lpVtbl->GetService(client, &IID_IAudioRenderClient, (void**)&render), "WASAPI failed to get AudioRenderClient service");
        IF_NULL_RESTART (render, "Failed to get AudioRenderClient even though GetService returned S_OK");
        defer {render->lpVtbl->Release(render);}

        DO_OR_RESTART (client->lpVtbl->Start(client), "WASAPI failed to start audio client");

        uint32_t format = wave_format.Format.wFormatTag;
        if (format == WAVE_FORMAT_EXTENSIBLE) {
            if (IsEqualGUID(&wave_format.SubFormat, &KSDATAFORMAT_SUBTYPE_PCM))
                format = WAVE_FORMAT_PCM;
            else if (IsEqualGUID(&wave_format.SubFormat, &KSDATAFORMAT_SUBTYPE_IEEE_FLOAT))
                format = WAVE_FORMAT_IEEE_FLOAT;
            else {
                format = 0;
                LOG ("Unsupported wave format: %d", wave_format.SubFormat);
            }
        }
        else {
            wave_format.Samples.wValidBitsPerSample = wave_format.Format.wBitsPerSample;
        }

        switch (format) {
            case WAVE_FORMAT_PCM: {
                LOG ("Entering SoundLoop_int");
                SoundLoop_int (wave_format, event_handle, render, frames_per_period);
            } break;
            case WAVE_FORMAT_IEEE_FLOAT: {
                LOG ("Entering SoundLoop_float");
                SoundLoop_float (wave_format, event_handle, render, frames_per_period);
            } break;
            // Handle the other formats at some point?
            default: LOG ("Not entering a sound loop - unsupported format"); break;
        }
    }

    LOG ("Sound thread exiting");

	return 0;
}

// Any number of identical channels, 16-bit signed integer
#define funcname__ SoundLoop_int
#define type__ int32_t
#include "sound_func.c"
#undef type__
#undef funcname__
// Any number of identical channels, float
#define funcname__ SoundLoop_float
#define type__ float
#include "sound_func.c"
#undef type__
#undef funcname__
