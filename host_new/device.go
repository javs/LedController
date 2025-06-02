package device

import (
	"encoding/binary"
	"fmt"
	"sync"
	"time"

	"github.com/sstallion/go-hid"
)

const USBVID = 0x16c0
const USBPID = 0x05df

type Device struct {
	hidDevice *hid.Device
	mutex     sync.Mutex
}

type usbMessageType uint8

const (
	usbMessageTypeGetLEDState usbMessageType = iota
	usbMessageTypeSetLEDState
	usbMessageTypeSetLightSensorRange
)

type UsbMessage struct {
	MsgType     usbMessageType
	CurrentTime uint64
	On          bool
	User        bool
	AutoLevels  bool
	Warm        uint16
	Cool        uint16
}

func Find() (*Device, error) {
	if err := hid.Init(); err != nil {
		return nil, fmt.Errorf("HID Init: %w", err)
	}

	d, err := hid.OpenFirst(USBVID, USBPID)
	if err != nil {
		return nil, fmt.Errorf("HID Open: %w", err)
	}

	return &Device{d, sync.Mutex{}}, nil
}

func (d *Device) GetState() (*UsbMessage, error) {
	buf := []byte{0} // report id
	buf, err := binary.Append(buf, binary.LittleEndian, &UsbMessage{MsgType: usbMessageTypeGetLEDState})
	if err != nil {
		return nil, fmt.Errorf("msg pack: %w", err)
	}

	d.mutex.Lock()
	defer d.mutex.Unlock()

	if _, err := d.hidDevice.Write(buf); err != nil {
		return nil, fmt.Errorf("HID Write: %w", err)
	}

	if _, err := d.hidDevice.ReadWithTimeout(buf, 1500*time.Millisecond); err != nil {
		return nil, fmt.Errorf("HID Read: %w", err)
	}

	var result UsbMessage
	if _, err := binary.Decode(buf, binary.LittleEndian, &result); err != nil {
		return nil, fmt.Errorf("response Decode: %w", err)
	}

	return &result, nil
}
