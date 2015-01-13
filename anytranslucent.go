// 7 january 2015
package main

import (
	"fmt"
	"os"
	"path/filepath"
	"image"
	_ "image/png"
)

func walk(path string, info os.FileInfo, err error) error {
	if err != nil {
		return err
	}
	if info.IsDir() {
		return nil
	}
	f, err := os.Open(path)
	if err != nil {
		return err
	}
	defer f.Close()
	img, _, err := image.Decode(f)
	if err != nil {	// probably not a valid image
		return nil
	}
	n := 0
	for y := 0; y < img.Bounds().Dy(); y++ {
		for x := 0; x < img.Bounds().Dx(); x++ {
			_, _, _, a := img.At(x, y).RGBA()
			if a != 0 && a != 0xFFFF {
				n++
			}
		}
	}
	if n != 0 {
		fmt.Printf("%d/%d %s\n", n, img.Bounds().Dx() * img.Bounds().Dy(), path)
	}
	return nil
}

func main() {
	err := filepath.Walk(os.Args[1], walk)
	if err != nil {
		panic(err)
	}
}
