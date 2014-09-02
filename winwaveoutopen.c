// 2 september 2014
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
	HWAVEOUT wo;
	HANDLE event;
	WAVEFORMATEX fmt;
	MMRESULT err;

	event = CreateEvent(NULL, TRUE, TRUE, NULL);			// start off signaled just in case
	if (event == NULL) {
		fprintf(stderr, "CreateEvent() failed (last error %d)\n", GetLastError());
		return 3;
	}

	ZeroMemory(&fmt, sizeof (WAVEFORMATEX));
	fmt.wFormatTag = WAVE_FORMAT_PCM;
	fmt.nChannels = 1;
	fmt.nSamplesPerSec = 44100;
	fmt.wBitsPerSample = 8;
	fmt.nBlockAlign = fmt.nChannels * fmt.wBitsPerSample;
	fmt.nAvgBytesPerSec = fmt.nSamplesPerSec * fmt.nBlockAlign;
	fmt.cbSize = 0;
	err = waveOutOpen(&wo, WAVE_MAPPER, &fmt,
		(DWORD_PTR) event, 0,
		CALLBACK_EVENT | WAVE_FORMAT_DIRECT);
	if (err != MMSYSERR_NOERROR) {
		WCHAR errmsg[MAXERRORLENGTH + 1];
		MMRESULT converr;

		converr = waveOutGetErrorTextW(err, errmsg, MAXERRORLENGTH + 1);
		if (converr != MMSYSERR_NOERROR) {
			fprintf(stderr, "open error %x (message conversion error %x)\n", err, converr);
			return 2;
		}
		fwprintf(stderr, L"open error: %s\n", errmsg);
		return 1;
	}
	fprintf(stderr, "open successful\n");
	return 0;
}
