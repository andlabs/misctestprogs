// 3 february 2015
#define UNICODE
#define _UNICODE
#define STRICT
#define STRICT_TYPED_ITEMIDS
#define CINTERFACE
#define COBJMACROS
#define WINVER 0x0600
#define _WIN32_WINNT 0x0600
#define _WIN32_WINDOWS 0x0600
#define _WIN32_IE 0x0700
#define NTDDI_VERSION 0x06000000
#include <windows.h>
#include <mmdeviceapi.h>
#include <propsys.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

// TODO FIGURE OUT HOW TO BUILD
// - mingw needs to have mmdevapi added
// - msvc fails spectacularly ??????????????????

void die(char *fmt, HRESULT hr, ...)
{
	va_list ap;

	va_start(ap, hr);
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, ": %d\n", hr);
	va_end(ap);
	exit(EXIT_FAILURE);
}

const char *formFactorNames[EndpointFormFactor_enum_count] = {
	"RemoteNetworkDevice",
	"Speakers",
	"LineLevel",
	"Headphones",
	"Microphone",
	"Headset",
	"Handset",
	"UnknownDigitalPassthrough",
	"SPDIF",
	"DigitalAudioDisplayDevice",
	"UnknownFormFactor",
};

void printDevice(IMMDevice *device, UINT iDevice)
{
	HRESULT hr;
	IPropertyStore *ps;
	PROPVARIANT pv;

	hr = IMMDevice_OpenPropertyStore(device, STGM_READ, &ps);
	if (hr != S_OK)
		die("error getting device %d property store", hr, iDevice);

	printf("device %d:\n", iDevice);

#define GETPROP(key) IPropertyStore_GetValue(ps, &key, &pv);

	hr = GETPROP(PKEY_Device_FriendlyName);
	if (hr != S_OK && hr != INPLACE_S_TRUNCATED)
		die("error getting friendly name of device %d", hr, iDevice);
	printf("friendly name: %S\n", pv.pwszVal);
	// TODO free?

	hr = GETPROP(PKEY_AudioEndpoint_FormFactor);
	if (hr != S_OK && hr != INPLACE_S_TRUNCATED)
		die("error getting endpoint form factor of device %d", hr, iDevice);
	printf("endpoint form factor: ");
	if (pv.uintVal < 0 || pv.uintVal >= EndpointFormFactor_enum_count)
		printf("unknown form factor type %d", pv.uintVal);
	else
		printf("%s", formFactorNames[pv.uintVal]);
	printf("\n");
	// TODO PropVariantClear()?

	IPropertyStore_Release(ps);
}

int main(void)
{
	HRESULT hr;
	IMMDeviceEnumerator *devenum;
	IMMDeviceCollection *devices;
	UINT i, nDevices;

	hr = CoInitialize(NULL);
	if (hr != S_OK && hr != S_FALSE)
		die("error initializing COM", hr);

	hr = CoCreateInstance(&CLSID_MMDeviceEnumerator,
		NULL, CLSCTX_ALL,
		&IID_IMMDeviceEnumerator,
		(LPVOID *) (&devenum));
	if (hr != S_OK)
		die("error creating device enumerator", hr);
	hr = IMMDeviceEnumerator_EnumAudioEndpoints(devenum, eAll, DEVICE_STATEMASK_ALL, &devices);
	if (hr != S_OK)
		die("error creating device list", hr);

	hr = IMMDeviceCollection_GetCount(devices, &nDevices);
	if (hr != S_OK)
		die("error getting device count", hr);
	for (i = 0; i < nDevices; i++) {
		IMMDevice *device;

		hr = IMMDeviceCollection_Item(devices, i, &device);
		if (hr != S_OK)
			die("error getting device %d", hr, i);
		printDevice(device, i);
		IMMDevice_Release(device);
		printf("\n");
	}

	IMMDeviceCollection_Release(devices);
	IMMDeviceEnumerator_Release(devenum);
	CoUninitialize();
	return 0;
}
