package main

import (
	device "leds/host"
	"log"
)

func main() {
	d, err := device.Find()
	if err != nil {
		log.Fatalln("Failed to open device: ", err)
	}

	defer d.Close()

	err = d.Toggle(true)
	log.Println(d.GetState())
	log.Println(err)
}
