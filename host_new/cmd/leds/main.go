package main

import (
	"fmt"
	device "leds/host"
	"log"
	"strings"

	"github.com/alecthomas/kong"
)

type CLI struct {
	State string `arg:"" optional:"" help:"on|off. Set idle state. Omit to just show state."`
	Mode  string `arg:"" optional:"" help:"user|auto. With STATE: set the user toggle instead."`
}

var cli CLI

func (c *CLI) Validate() error {
	c.State = strings.ToLower(c.State)
	c.Mode = strings.ToLower(c.Mode)

	if c.State != "" && c.State != "on" && c.State != "off" {
		return fmt.Errorf("STATE must be \"on\" or \"off\", got %q", c.State)
	}
	if c.Mode != "" && c.Mode != "user" && c.Mode != "auto" {
		return fmt.Errorf("MODE must be \"user\" or \"auto\", got %q", c.Mode)
	}
	return nil
}

func main() {
	log.SetFlags(0) // drop timestamps; this is a CLI, not a daemon
	kong.Parse(&cli, kong.Description("Query or toggle device state."))

	if err := run(); err != nil {
		log.Fatalln("error:", err)
	}
}

func run() error {
	d, err := device.Find()
	if err != nil {
		return fmt.Errorf("failed to open device: %w", err)
	}
	defer d.Close()

	on := cli.State == "on"
	switch {
	case cli.Mode != "":
		err = d.ToggleUser(on, cli.Mode == "user")
	case cli.State != "":
		err = d.ToggleIdle(on)
	}
	if err != nil {
		return err
	}

	st, err := d.GetState()
	if err != nil {
		return fmt.Errorf("failed to read state: %w", err)
	}
	log.Print(st)

	return nil
}
