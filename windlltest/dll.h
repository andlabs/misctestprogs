// 5 march 2015
#define UNICODE
#define _UNICODE
#define STRICT
#define STRICT_TYPED_ITEMIDS
#define WINVER 0x0501
#define _WIN32_WINNT 0x0501
#define _WIN32_WINDOWS 0x0501           /* according to Microsoft's winperf.h */
#define _WIN32_IE 0x0600                        /* according to Microsoft's sdkddkver.h */
#define NTDDI_VERSION 0x05010000        /* according to Microsoft's sdkddkver.h */
#include <windows.h>
#include <stdio.h>

#define windowClass L"windowClass"
extern __declspec(dllexport) ATOM registerClass(void);
