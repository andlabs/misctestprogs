// 11 april 2015
package main

import (
	"fmt"
	"os"
	"bufio"
	"time"
)

const dateform = "[Monday, January 2, 2006] [3:04:05 PM"

func main() {
	scanner := bufio.NewScanner(os.Stdin)
	for scanner.Scan() {
		b := scanner.Bytes()
		if len(b) == 0 || b[0] != '[' {
			continue
		}
		i := 0
		n := 0
		for ; i < len(b); i++ {
			if b[i] == ' ' || b[i] == ']' {
				n++
				if n == 7 {
					break
				}
			}
		}
		t, err := time.Parse(dateform, string(b[:i]))
		if err != nil {
			panic(err)
		}
		if t.Month() != time.March {
			continue
		}
		fmt.Println(string(b))
	}
	if err := scanner.Err(); err != nil {
		panic(err)
	}
}
