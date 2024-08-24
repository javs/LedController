#pragma once

#include "drivers/I2C.h"

/**
 * APDS 9960 Light and Gesture Sensor Minimal Driver.
 * Data Sheet: https://docs.broadcom.com/doc/AV02-4191EN
 * 
 * Based on Adafruit's CircuitPython driver
 * https://github.com/adafruit/Adafruit_CircuitPython_APDS9960/blob/main/adafruit_apds9960/apds9960.py
 */
class APDS9960Driver {
    static const uint8_t I2C_ADDRESS = 0x39;

    // Registers
    // 0x00 - 0x7F RAM
    static const uint8_t REG_ENABLE     = 0x80; //!< Reset 0x00 - R/W Enable states and interrupts
    static const uint8_t REG_ATIME      = 0x81; //!< Reset 0xFF - R/W ADC integration time
    static const uint8_t REG_WTIME      = 0x83; //!< Reset 0xFF - R/W Wait time (non-gesture)
    static const uint8_t REG_AILTL      = 0x84; //!< Reset --   - R/W ALS interrupt low threshold low byte
    static const uint8_t REG_AILTH      = 0x85; //!< Reset --   - R/W ALS interrupt low threshold high byte
    static const uint8_t REG_AIHTL      = 0x86; //!< Reset 0x00 - R/W ALS interrupt high threshold low byte
    static const uint8_t REG_AIHTH      = 0x87; //!< Reset 0x00 - R/W ALS interrupt high threshold high byte
    static const uint8_t REG_PILT       = 0x89; //!< Reset 0x00 - R/W Proximity interrupt low threshold
    static const uint8_t REG_PIHT       = 0x8B; //!< Reset 0x00 - R/W Proximity interrupt high threshold
    static const uint8_t REG_PERS       = 0x8C; //!< Reset 0x00 - R/W Interrupt persistence filters (non-gesture)
    static const uint8_t REG_CONFIG1    = 0x8D; //!< Reset 0x60 - R/W Configuration register one
    static const uint8_t REG_PPULSE     = 0x8E; //!< Reset 0x40 - R/W Proximity pulse count and length
    static const uint8_t REG_CONTROL    = 0x8F; //!< Reset 0x00 - R/W Gain control
    static const uint8_t REG_CONFIG2    = 0x90; //!< Reset 0x01 - R/W Configuration register two
    static const uint8_t REG_ID         = 0x92; //!< Reset ID   - R   Device ID
    static const uint8_t REG_STATUS     = 0x93; //!< Reset 0x00 - R   Device status
    static const uint8_t REG_CDATAL     = 0x94; //!< Reset 0x00 - R   Low byte of clear channel data
    static const uint8_t REG_CDATAH     = 0x95; //!< Reset 0x00 - R   High byte of clear channel data
    static const uint8_t REG_RDATAL     = 0x96; //!< Reset 0x00 - R   Low byte of red channel data
    static const uint8_t REG_RDATAH     = 0x97; //!< Reset 0x00 - R   High byte of red channel data
    static const uint8_t REG_GDATAL     = 0x98; //!< Reset 0x00 - R   Low byte of green channel data
    static const uint8_t REG_GDATAH     = 0x99; //!< Reset 0x00 - R   High byte of green channel data
    static const uint8_t REG_BDATAL     = 0x9A; //!< Reset 0x00 - R   Low byte of blue channel data
    static const uint8_t REG_BDATAH     = 0x9B; //!< Reset 0x00 - R   High byte of blue channel data
    static const uint8_t REG_PDATA      = 0x9C; //!< Reset 0x00 - R   Proximity data
    static const uint8_t REG_POFFSET_UR = 0x9D; //!< Reset 0x00 - R/W Proximity offset for UP and RIGHT photodiodes
    static const uint8_t REG_POFFSET_DL = 0x9E; //!< Reset 0x00 - R/W Proximity offset for DOWN and LEFT photodiodes
    static const uint8_t REG_CONFIG3    = 0x9F; //!< Reset 0x00 - R/W Configuration register three
    static const uint8_t REG_GPENTH     = 0xA0; //!< Reset 0x00 - R/W Gesture proximity enter threshold
    static const uint8_t REG_GEXTH      = 0xA1; //!< Reset 0x00 - R/W Gesture exit threshold
    static const uint8_t REG_GCONF1     = 0xA2; //!< Reset 0x00 - R/W Gesture configuration one
    static const uint8_t REG_GCONF2     = 0xA3; //!< Reset 0x00 - R/W Gesture configuration two
    static const uint8_t REG_GOFFSET_U  = 0xA4; //!< Reset 0x00 - R/W Gesture UP offset register
    static const uint8_t REG_GOFFSET_D  = 0xA5; //!< Reset 0x00 - R/W Gesture DOWN offset register
    static const uint8_t REG_GOFFSET_L  = 0xA7; //!< Reset 0x00 - R/W Gesture LEFT offset register
    static const uint8_t REG_GOFFSET_R  = 0xA9; //!< Reset 0x00 - R/W Gesture RIGHT offset register
    static const uint8_t REG_GPULSE     = 0xA6; //!< Reset 0x40 - R/W Gesture pulse count and length
    static const uint8_t REG_GCONF3     = 0xAA; //!< Reset 0x00 - R/W Gesture configuration three
    static const uint8_t REG_GCONF4     = 0xAB; //!< Reset 0x00 - R/W Gesture configuration four
    static const uint8_t REG_GFLVL      = 0xAE; //!< Reset 0x00 - R   Gesture FIFO level
    static const uint8_t REG_GSTATUS    = 0xAF; //!< Reset 0x00 - R   Gesture status
    static const uint8_t REG_IFORCE     = 0xE4; //!< Reset 0x00 - W   Force interrupt (Requires "address accessing" transaction)
    static const uint8_t REG_PICLEAR    = 0xE5; //!< Reset 0x00 - W   Proximity interrupt clear (Requires "address accessing" transaction)
    static const uint8_t REG_CICLEAR    = 0xE6; //!< Reset 0x00 - W   ALS clear channel interrupt clear (Requires "address accessing" transaction)
    static const uint8_t REG_AICLEAR    = 0xE7; //!< Reset 0x00 - W   All non-gesture interrupts clear (Requires "address accessing" transaction)
    static const uint8_t REG_GFIFO_U    = 0xFC; //!< Reset 0x00 - R   Gesture FIFO UP value
    static const uint8_t REG_GFIFO_D    = 0xFD; //!< Reset 0x00 - R   Gesture FIFO DOWN value
    static const uint8_t REG_GFIFO_L    = 0xFE; //!< Reset 0x00 - R   Gesture FIFO LEFT value
    static const uint8_t REG_GFIFO_R    = 0xFF; //!< Reset 0x00 - R   Gesture FIFO RIGHT value

    static const uint8_t APDS9960_ID1 = 0xAB;
    static const uint8_t APDS9960_ID2 = 0xA8;

    enum REG_ENABLE_BITS {
        ENABLE_BIT_PON      = 1 << 0, //!< Power ON.
        ENABLE_BIT_AEN      = 1 << 1, //!< ALS Enable.
        ENABLE_BIT_PEN      = 1 << 2, //!< Proximity Detect Enable.
        ENABLE_BIT_WEN      = 1 << 3, //!< Wait Enable.
        ENABLE_BIT_AIEN     = 1 << 4, //!< ALS Interrupt Enable.
        ENABLE_BIT_PIEN     = 1 << 5, //!< Proximity Interrupt Enable.
        ENABLE_BIT_GEN      = 1 << 6, //!< Gesture Enable.
        // 7 Reserved. Write as 0.
    };

    static const uint8_t REG_CONFIG1_BASE = 0x40;
    static const uint8_t CONFIG1_BIT_WLONG = 1 << 1;
    
    mbed::I2C m_I2C;

    //! Read 1 byte register
    bool ReadReg(uint8_t reg, uint8_t& result);
    //! Read 2 byte register
    bool ReadReg(uint8_t op, uint16_t& result);
    //! Write 1 byte register
    bool WriteReg(uint8_t reg, uint8_t data);

public:
    APDS9960Driver(PinName sda, PinName scl);

    //! Detects and inits device
    bool Init(bool set_defaults = true);

    struct ColorSample {
        uint16_t clear;
        uint16_t red;
        uint16_t green;
        uint16_t blue;
    };

    bool GetColorData(ColorSample& color);
};
