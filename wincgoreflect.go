// 11 june 2014
package main

import (
	"fmt"
	"reflect"
)

// #define UNICODE
// #define _UNICODE
// #define STRICT
// #include <windows.h>
import "C"

func main() {
//	var subject C.LOGFONT
	var subject C.WNDCLASS

	t := reflect.TypeOf(subject)
	fmt.Printf("Name: %q\n", t.Name())
	fmt.Printf("Kind: %q\n", t.Kind())
	fmt.Printf("Field Count: %d\n", t.NumField())
	for i := 0; i < t.NumField(); i++ {
		field := t.Field(i)
		fmt.Printf("Field %d Name: %q\n", i, field.Name)
		fmt.Printf("Field %d Type: %v\n", i, field.Type)
		fmt.Printf("Field %d Type Kind: %q\n", i, field.Type.Kind())
	}
	fmt.Printf("String: %q", t.String())
}
