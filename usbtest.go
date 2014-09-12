// 11 september 2014
package main

import (
	"fmt"
	"unsafe"
)

// #cgo pkg-config: gusb
// /* epic fine print disclaimer alert!! */
// #define G_USB_API_IS_SUBJECT_TO_CHANGE
// #include <gusb.h>
// gpointer get(GPtrArray *arr, gint i)
// {
// 	return g_ptr_array_index(arr, i);
// }
import "C"

func main() {
	var err *C.GError = nil

	context := C.g_usb_context_new(&err);
	if context == nil {
		panic("new: " + C.GoString((*C.char)(unsafe.Pointer(err.message))))
	}
	devlist := C.g_usb_device_list_new(context)
	C.g_usb_device_list_coldplug(devlist)
	list := C.g_usb_device_list_get_devices(devlist)
	n := list.len
	fmt.Printf("%d devices\n", n);
	for i := 0; i < int(n); i++ {
		ptr := C.get(list, C.gint(i))
		dev := (*C.GUsbDevice)(unsafe.Pointer(ptr))
		name := (*C.char)(unsafe.Pointer(C.g_usb_device_get_platform_id(dev)))
		fmt.Printf("%d %s %x %x\n", i + 1, C.GoString(name),
			C.g_usb_device_get_vid(dev),
			C.g_usb_device_get_pid(dev))
	}
}
