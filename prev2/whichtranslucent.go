// 7 january 2015
package main

import (
	"fmt"
	"os"
	"image"
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
	for y := 0; y < img.Bounds().Dy(); y++ {
		for x := 0; x < img.Bounds().Dx(); x++ {
			_, _, _, a := img.At(x, y).RGBA()
			if a == 0 {
				fmt.Printf(" ")
			} else if a == 0xFFFF {
				fmt.Printf("#")
			} else {
				fmt.Printf("•")
			}
		}
		fmt.Printf("\n")
	}
}
