package main

import (
	"fmt"
	device "leds/host"
)

func main() {
	d, _ := device.Find()
	fmt.Println(d.GetState())
}
