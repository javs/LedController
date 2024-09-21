#include "apds9960_driver.hpp"

using namespace mbed;

APDS9960Driver::APDS9960Driver(PinName sda, PinName scl)
    : m_I2C(sda, scl)
{
}

bool APDS9960Driver::Init(bool set_defaults)
{
    uint8_t id;
    if (!ReadReg(REG_ID, id) || (id != APDS9960_ID1 && id != APDS9960_ID2))
        return false;
    
    if (set_defaults) {
        // Enable sensor, ALS and wait modules
        WriteReg(REG_ENABLE, ENABLE_BIT_PON | ENABLE_BIT_AEN | ENABLE_BIT_WEN);
        // Max wait time ~8s
        WriteReg(REG_WTIME, 0xFF);
        WriteReg(REG_CONFIG1, REG_CONFIG1_BASE | CONFIG1_BIT_WLONG);
        // Max color integration time
        WriteReg(REG_ATIME, 0xAF);
        // 4x color gain
        WriteReg(REG_CONTROL, 1); // TODO: extract
    }

    return true;
}

bool APDS9960Driver::ReadReg(uint8_t op, uint8_t &result)
{
    char buf = op;

    const auto i2c_op = I2C_ADDRESS << 1;
    if (m_I2C.write(i2c_op, &buf, 1) != 0)
        return false;

    if (m_I2C.read(i2c_op, &buf, 1) != 0)
        return false;
    
    result = buf;

    return true;
}

bool APDS9960Driver::ReadReg(uint8_t op, uint16_t& result)
{
    char buf[2] = {op};

    const auto i2c_op = I2C_ADDRESS << 1;
    if (m_I2C.write(i2c_op, &buf[0], 1) != 0)
        return false;

    if (m_I2C.read(i2c_op, &buf[0], 2) != 0)
        return false;

    result = buf[1] << 8 | buf[0];

    return true;
}

bool APDS9960Driver::WriteReg(uint8_t op, uint8_t data)
{
    char buf[2] = {op, data};

    const auto i2c_op = I2C_ADDRESS << 1;
    if (m_I2C.write(i2c_op, buf, 2) != 0)
        return false;

    return true;
}

bool APDS9960Driver::GetColorData(ColorSample& color)
{
    return
        ReadReg(REG_CDATAL, color.clear) &&
        ReadReg(REG_RDATAL, color.red) &&
        ReadReg(REG_GDATAL, color.green) &&
        ReadReg(REG_BDATAL, color.blue);
}
