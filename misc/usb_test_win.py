#!/usr/bin/env python3

import threading
import struct
from pywinusb import hid
from enum import Enum


class MessageType(Enum):
    GetState = 0
    SetState = 1


def on_data(data):
    print(f"Got message {data}")
    (_, msg, on, warm, cool) = struct.unpack('<BB?HH', bytearray(data))

    warm = (warm / (2 ** 16)) * 100.0
    cool = (cool / (2 ** 16)) * 100.0

    print(f"{MessageType(msg)} on: {on} warm: {warm:.2f} cool: {cool:.2f}")
    done.set()


def send_set_command(report, on, warm, cool):
    warm /= 100 / (2 ** 16)
    cool /= 100 / (2 ** 16)
    # The first byte is the report ID which must be 0
    buffer = struct.pack('<BB?HH', 0, MessageType.SetState.value, on, int(warm), int(cool))
    print(f"Sending {buffer}")
    report.set_raw_data(buffer)
    report.send()


def send_get_command(report):
    # The first byte is the report ID which must be 0
    # Fields are ignored for this msg
    buffer = struct.pack('<BB?HH', 0, MessageType.GetState.value, False, 0, 0)
    report.set_raw_data(buffer)
    report.send()


all_hid_devices = hid.find_all_hid_devices()
mbed_devices = [d for d in all_hid_devices if "mbed" in d.vendor_name]

if not mbed_devices:
    raise ValueError("No HID devices found")

done = threading.Event()

mbed_devices[0].open()
mbed_devices[0].set_raw_data_handler(on_data)

out_report = mbed_devices[0].find_output_reports()
send_set_command(out_report[0], True, 40.0, 15.0)
# send_get_command(out_report[0])

done.wait(5)
