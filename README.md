# USB LED Controller

USB controller for my monitor LED strip.

## Device Firmware

* Warm/Cool/Brightness control
* Persistent storage in flash
* Timed protection from extended high power use
* Control via a USB HID device
* Mbed 6.x
* STM32F3

## Host App

* LED warmth control based on time of day.
* Match LEDs to PC Sleep and Inactivity states.
* Tray Icon & Flyout
* WinUI 3 XAML UI
* Windows App SDK Packaged App

## Development

### Testing

#### Linux

    # Turn off
    sudo hidapitester --vidpid 16C0/05DF -l 6 --open --send-output 1,0,0x10,0x10,0xFF,0xFF --read-input

### Device

#### Container

    podman build \
        --tag leds-device-builder:latest \
        device/docker

    podman run \
        --rm -it \
        --name leds-device \
        -v .:/src \
        --device /dev/ttyACM0 \
        --group-add keep-groups \
        leds-device-builder

#### ST-Link

1. Jumpers in normal position:
    1. JP1 removed.
    1. ST-LINK both populated.
    1. JP5 populated in 2-3 (E5V).
1. Connect main USB port & power supply.
1. LD3 solid, LD1 blinking.
1. Connect USB debug CN1.
1. Device reset and LD1 solid.

#### Upload

Copy binary to USB storage device.

#### Console UART

    mbed-tools sterm -p /dev/ttyACM0

