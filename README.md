# USB LED Controller

USB controller for my monitor LED strip.

## Device Firmware

* Warm/Cool/Brightness control
* Persistent storage in flash
* Timed protection from extended high power use
* Control via a USB HID device
* Mbed 6.x
* STM32F3
* Light and Gesture Sensor: APDS 9960

## Host App

* LED warmth control based on time of day.
* Match LEDs to PC Sleep and Inactivity states.
* Tray Icon & Flyout
* WinUI 3 XAML UI
* Windows App SDK Packaged App

## Development

### Testing

#### Linux

##### Permissions

    # Setup permissions
    sudo tee /etc/udev/rules.d/60-leds.rules <<'STOP'
    # HID for LEDs (not used in container)
    KERNEL=="hidraw*", SUBSYSTEM=="hidraw", ATTRS{idVendor}=="16c0", ATTRS{idProduct}=="05df", \
        TAG+="uaccess"

    # STLink v2
    # From https://github.com/stlink-org/stlink/blob/master/config/udev/rules.d/49-stlinkv2-1.rules
    # Used in container, no acl (uaccess)
    SUBSYSTEMS=="usb", ATTRS{idVendor}=="0483", ATTRS{idProduct}=="374a", \
        MODE:="0666", \
        SYMLINK+="stlinkv2-1_%n"

    SUBSYSTEMS=="usb", ATTRS{idVendor}=="0483", ATTRS{idProduct}=="374b", \
        MODE:="0666", \
        SYMLINK+="stlinkv2-1_%n"

    SUBSYSTEMS=="usb", ATTRS{idVendor}=="0483", ATTRS{idProduct}=="3752", \
        MODE:="0666", \
        SYMLINK+="stlinkv2-1_%n"
    STOP

    # Reload
    sudo udevadm control --reload-rules && sudo udevadm trigger

##### Tests

    # Turn on
    hidapitester --vidpid 16C0/05DF --open \
        -l 17 --send-output 0,1,0x90,0x12,0xE7,0x66,0x00,0x00,0x00,0x00,1,0,1,0xFF,0xFF,0xFF,0xFF \
        -l 16 --read-input 0

    # Get
    hidapitester --vidpid 16C0/05DF --open \
        -l 17 --send-output 0,0,0x90,0x12,0xE7,0x66,0x00,0x00,0x00,0x00,1,0,1,0xFF,0xFF,0xFF,0xFF \
        -l 16 --read-input 0

    # Notifications
    hidapitester --vidpid 16C0/05DF --open \
        -l 16 -t 10000 --read-input 0

    # Set Light Sensor Range
    hidapitester --vidpid 16C0/05DF --open \
        -l 17 --send-output 0,2,0x00,0x80,0x00,0x40,0,0,0,0,0,0,0,0,0,0,0 \
        -l 16 --read-input 0

### Device

#### Container

Launch and build directly from vscode.

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

Use flash-leds target, or copy to storage device.

#### Console UART

    screen /dev/ttyACM0

### Links

* [Nucleo Pinout](https://os.mbed.com/platforms/ST-Nucleo-F303RE/)
* [STM32F303RE Manual](https://www.st.com/resource/en/datasheet/stm32f303re.pdf)
* [APDS 9960 Adafruit Docs](https://learn.adafruit.com/adafruit-apds9960-breakout/circuitpython)
