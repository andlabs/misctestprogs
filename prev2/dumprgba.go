// 7 january 2015
package main

import (
	"fmt"
	"os"
	"image"
	"image/draw"
	_ "image/png"
)

func main() {
	f, err := os.Open(os.Args[1])
	if err != nil {
		panic(err)
	}
	defer f.Close()
	img, _, err := image.Decode(f)
	if err != nil {
		panic(err)
	}
	realimg := image.NewRGBA(img.Bounds())
	draw.Draw(realimg, realimg.Bounds(), img, image.ZP, draw.Src)
	n := 0
	fmt.Printf("\t")
	for y := 0; y < realimg.Bounds().Dy(); y++ {
		for x := 0; x < realimg.Bounds().Dx(); x++ {
			c := realimg.RGBAAt(x, y)
			co := uint32(0)
			co |= uint32(c.A) << 24
			co |= uint32(c.R) << 16
			co |= uint32(c.G) << 8
			co |= uint32(c.B)
			fmt.Printf("0x%X,", co)
			n++
			if n % 16 == 0 {
				fmt.Printf("\n\t")
			} else {
				fmt.Printf(" ")
			}
		}
	}
}
