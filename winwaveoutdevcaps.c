// 1 september 2014
#define UNICODE
#define _UNICODE
#define STRICT
#define STRICT_TYPED_ITEMIDS
// get Windows version right; right now Windows XP
#define WINVER 0x0501
#define _WIN32_WINNT 0x0501
#define _WIN32_WINDOWS 0x0501		/* according to Microsoft's winperf.h */
#define _WIN32_IE 0x0600			/* according to Microsoft's sdkddkver.h */
#define NTDDI_VERSION 0x05010000	/* according to Microsoft's sdkddkver.h */
#include <windows.h>
#include <stdio.h>

int main(void)
{
	MMRESULT err;
	WAVEOUTCAPS caps;

	ZeroMemory(&caps, sizeof (WAVEOUTCAPS));
	err = waveOutGetDevCaps((DWORD_PTR) WAVE_MAPPER, &caps, sizeof (WAVEOUTCAPS));
	if (err != MMSYSERR_NOERROR) {
		fprintf(stderr, "mmsys err %x\n", err);
		return 1;
	}

	printf("formats: ");
#define FORMATS(x) if((caps.dwFormats & x) != 0) printf("%s ", #x);
	FORMATS(WAVE_FORMAT_1M08);
	FORMATS(WAVE_FORMAT_1M16);
	FORMATS(WAVE_FORMAT_1S08);
	FORMATS(WAVE_FORMAT_1S16);
	FORMATS(WAVE_FORMAT_2M08);
	FORMATS(WAVE_FORMAT_2M16);
	FORMATS(WAVE_FORMAT_2S08);
	FORMATS(WAVE_FORMAT_2S16);
	FORMATS(WAVE_FORMAT_4M08);
	FORMATS(WAVE_FORMAT_4M16);
	FORMATS(WAVE_FORMAT_4S08);
	FORMATS(WAVE_FORMAT_4S16);
	FORMATS(WAVE_FORMAT_96M08);
	FORMATS(WAVE_FORMAT_96M16);
	FORMATS(WAVE_FORMAT_96S08);
	FORMATS(WAVE_FORMAT_96S16);
	printf("\n");

	printf("channels: %d\n", caps.wChannels);

	printf("supports: ");
#define SUPPORTS(x) if((caps.dwSupport & x) != 0) printf("%s ", #x);
	SUPPORTS(WAVECAPS_LRVOLUME);
	SUPPORTS(WAVECAPS_PITCH);
	SUPPORTS(WAVECAPS_PLAYBACKRATE);
	SUPPORTS(WAVECAPS_SYNC);
	SUPPORTS(WAVECAPS_VOLUME);
	SUPPORTS(WAVECAPS_SAMPLEACCURATE);
	printf("\n");

	return 0;
}
