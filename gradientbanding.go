// 18 february 2015
package main

import (
	"fmt"
	"net/http"
	"image"
	_ "image/png"
)

func main() {
	resp, err := http.Get("https://i-msdn.sec.s-msft.com/dynimg/IC163990.png")
	if err != nil {
		panic(err)
	}
	defer resp.Body.Close()
	png, _, err := image.Decode(resp.Body)
	if err != nil {
		panic(err)
	}
	b := png.Bounds()
	for y := b.Min.Y; y < b.Max.Y; y++ {
		c := png.At((b.Max.X - b.Min.X) / 2, y)
		fmt.Printf("%v\n", c)
	}
}
