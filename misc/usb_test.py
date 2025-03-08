#!/usr/bin/env python3

import datetime
import struct
import sys
import time
import hid # todo: move to the one in openrgb
from enum import Enum
from dataclasses import dataclass

vid = 0x16c0
pid = 0x05df


# from common/led_device.h
@dataclass
class LEDState:
    class MessageType(Enum):
        GetLEDState = 0
        SetLEDState = 1
        SetLightSensorRange = 2

    report_id: int = 0
    msg_type: MessageType = MessageType.GetLEDState
    current_time: int = 0
    on: bool = False
    user: bool = False
    auto_levels: bool = False
    warm: int = 0
    cool: int = 0

    # Define the struct format string (little-endian)
    STRUCT_FORMAT = '<BBQ???HH'

    def get_time_for_device(self) -> int:
        # The device use local time, no timezones
        # calculate the utc offset
        current_utc = datetime.datetime.now(datetime.UTC).replace(tzinfo=None)
        local_time = datetime.datetime.now()
        utc_offset = (local_time - current_utc).total_seconds()

        return int(time.time() + utc_offset)

    def pack(self) -> bytes:
        # warm = self._clamp_uint16(warm / 100 * (2 ** 16))
        # cool = self._clamp_uint16(cool / 100 * (2 ** 16))

        """Packs the data into a binary format using struct.pack."""
        return struct.pack(
            self.STRUCT_FORMAT,
            self.report_id,
            self.msg_type.value,
            self.get_time_for_device(),
            self.on,
            self.user,
            self.auto_levels,
            self.warm,
            self.cool
        )

    @classmethod
    def unpack(cls, data: bytes) -> 'LEDState':
        """Unpacks a binary format into an LEDState instance using struct.unpack."""
        data = bytearray(data)
        data.insert(0, 0)  # add report id to keep the structure
        unpacked_data = struct.unpack(cls.STRUCT_FORMAT, data)
        # warm = (warm / (2 ** 16)) * 100.0
        # cool = (cool / (2 ** 16)) * 100.0
        return cls(*unpacked_data)
    
    @classmethod
    def _clamp_uint16(cls, val: int) -> int:
        return cls._clamp(int(val), 0, 2 ** 16 -1)

    @classmethod
    def _clamp(cls, val: int, minval: int, maxval: int) -> int:
        if val < minval: return minval
        if val > maxval: return maxval
        return val


def send_state_command(device, data: LEDState):
    device.write(data.pack())

    response = device.read(30, 500)
    state = LEDState.unpack(response)

    print(f"{state}")


with hid.Device(vid, pid) as device:
    if len(sys.argv) == 1:
        send_state_command(device, LEDState(0, LEDState.MessageType.GetLEDState))
    elif len(sys.argv) == 2:
        if sys.argv[1] == 'on':
            on = True
        else:
            on = False
        send_state_command(device, LEDState(0, LEDState.MessageType.SetLEDState, 0, on, False, True, 0, 0))
    # else:
    #    print(f"Usage: {sys.argv[0]} <warm 0-100> <cool 0-100>")
    #    sys.exit(1)
