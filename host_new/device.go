package device

import (
	"encoding/binary"
	"errors"
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

var ErrDeviceNotFound = errors.New("device not found")

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

// Find returns the LED device if present, ErrDeviceNotFound or another error on failure.
func Find() (*Device, error) {
	if err := hid.Init(); err != nil {
		return nil, fmt.Errorf("HID Init: %w", err)
	}

	d, err := hid.OpenFirst(USBVID, USBPID)
	if err != nil {
		return nil, errors.Join(ErrDeviceNotFound, err)
	}

	return &Device{d, sync.Mutex{}}, nil
}

// GetState returns the current state of the device, or an error.
func (d *Device) GetState() (*UsbMessage, error) {
	msg := &UsbMessage{MsgType: usbMessageTypeGetLEDState}

	return d.sendUsbMessage(msg)
}

// SetState sets the current state of the device, or returns an error.
func (d *Device) SetState(state *UsbMessage) (*UsbMessage, error) {
	// FIXME: fix interface
	state.MsgType = usbMessageTypeSetLEDState
	return d.sendUsbMessage(state)
}

// ToggleUser turns leds on/off and sets user driven intent. \see ToggleIdle for automatic intent
func (d *Device) ToggleUser(on bool, user bool) error {
	// FIXME: rework device msg
	state, err := d.GetState()

	if err != nil {
		return fmt.Errorf("GetState: %w", err)
	}

	state.On = on
	state.User = user

	_, err = d.SetState(state)

	if err != nil {
		return fmt.Errorf("SetState: %w", err)
	}

	return nil
}

// ToggleIdle indicates the seat is idle and turns off/on the leds unless overridden by a user.
func (d *Device) ToggleIdle(on bool) error {
	// FIXME: rework device msg
	state, err := d.GetState()

	if err != nil {
		return fmt.Errorf("GetState: %w", err)
	}

	// user overriden, dont change state
	if state.User {
		return nil
	}

	state.On = on

	_, err = d.SetState(state)

	if err != nil {
		return fmt.Errorf("SetState: %w", err)
	}

	return nil
}

// sendUsbMessage sends a UsbMessage to the device and returns the response, or an error.
func (d *Device) sendUsbMessage(msg *UsbMessage) (*UsbMessage, error) {
	buf := []byte{0} // report id
	buf, err := binary.Append(buf, binary.LittleEndian, msg)
	if err != nil {
		return nil, fmt.Errorf("msg pack: %w", err)
	}

	if err := d.readWriteUsb(buf); err != nil {
		return nil, fmt.Errorf("readWriteUsb: %w", err)
	}

	if _, err := binary.Decode(buf, binary.LittleEndian, msg); err != nil {
		return nil, fmt.Errorf("response Decode: %w", err)
	}

	return msg, nil
}

// readWriteUsb writes a message to the device, and then reads a message of the same length from it.
func (d *Device) readWriteUsb(msg []byte) error {
	d.mutex.Lock()
	defer d.mutex.Unlock()

	if _, err := d.hidDevice.Write(msg); err != nil {
		return fmt.Errorf("HID Write: %w", err)
	}

	if _, err := d.hidDevice.ReadWithTimeout(msg, 1500*time.Millisecond); err != nil {
		return fmt.Errorf("HID Read: %w", err)
	}

	return nil
}

// Close closes the device,
func (d *Device) Close() error {
	if err := d.hidDevice.Close(); err != nil {
		return fmt.Errorf("HID Close: %w", err)
	}

	d.hidDevice = nil

	return nil
}

func (m *UsbMessage) String() string {
	return fmt.Sprintf("[type=%d time=%s on=%s user=%s auto=%s warm=%d cool=%d]",
		m.MsgType,
		time.Unix(int64(m.CurrentTime), 0).Format(time.TimeOnly),
		onOff(m.On), onOff(m.User), onOff(m.AutoLevels),
		m.Warm, m.Cool)
}

func onOff(b bool) string {
	if b {
		return "on"
	}
	return "off"
}
