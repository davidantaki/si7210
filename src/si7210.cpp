// File: si7210.cpp
// Author: David Antaki
// Date: 7/11/20
// License: This software is not open source and is copyrighted by David
// Antaki.
// Contents: A custom MBED I2C driver for the Silicon Labs Si7210 digital hall
// sensor

#include "si7210.h"

si7210::si7210(I2C *i2cBus, uint8_t addr, si7210_range_t r, si7210_magnet_t mag, si7210_mode_t m, Filter f)
{
    i2c = i2cBus;
    devAddr7Bit = addr;
    devAddr8Bit = addr << 1;
    range = r;
    magnet = mag;
    mode = m;
    filter = f;

    init();
}

si7210::~si7210() {}

void si7210::init()
{
    setMode(mode);
    setRange(range, magnet);
    setFilter(filter);
}

// Host command for reading an I2C register (from si7210 Datasheet):
// Note: the number of bits is in paren's (e.g. (8)=8bit)
// START(1) | DeviceAddress(7) | W(1) | ACK(1) | RegisterAddress(8) | ACK(1)
// | Sr=repeated start(1) | DeviceAddress(7) | R(1) | Data(8) | NACK(1) | STOP(1)
bool si7210::readRegister(uint8_t _reg, uint8_t *_returnedData)
{
    // Sends start bit.
    // Writes device address+write bit onto bus.
    // Writes the specific register address (address length=1 byte) of the
    // device to read from onto bus.
    // Repeated start is true (doesn't send stop bit)
    i2c->write(devAddr8Bit, (const char *)&_reg, 1, true);

    // Sends start bit.
    // Writes device address+read bit onto bus.
    // Reads 1 byte of data.
    // Sends stop bit.
    // Return ack or nack to host:
    // 0 on success (ack), nonzero on failure (nack).
    return i2c->read(devAddr8Bit, (char *)_returnedData, 1, false) == 0;
}

// Host command for writing an I2C register (from si7210 Datasheet):
// Note: the number of bits is in paren's (e.g. (8)=8bit)
// START(1) | DeviceAddress(7) | W(1) | ACK(1) | RegisterAddress(8) | ACK(1)
// | Data(8) | ACK(1) | STOP(1)
bool si7210::writeRegister(uint8_t _reg, uint8_t _data)
{
    uint8_t buffer[2] = {_reg, _data};

    // Writes DeviceAddress onto bus, forces bottom bit/LSB to 0 to indicate
    // write.
    // Sends 2 bytes of data that are contained in the buffer[]:
    //      1. The specific register address to write to/
    //      2. The byte to write to that register.
    //
    // The 1 write command is the same as these 2 write commands:
    //      i2c->write(devAddr8Bit, (const char *)_reg, 1, false);
    //      i2c->write(devAddr8Bit, (const char *)_data, 1, false);
    return i2c->write(devAddr8Bit, (const char *)buffer, 2, false) == 0;
}

uint8_t si7210::getChipId()
{
    uint8_t temp;
    readRegister(REG_0XC0, &temp);
    return (temp >> 4); // Bit shift to get bits 4:7 which hold the chip ID
}

uint8_t si7210::getRevId()
{
    uint8_t temp;
    readRegister(REG_0XC0, &temp);

    // (0x1 << 4) = 11110000
    // ~(0x1 << 4) = 00001111
    // temp & (~(0x1 << 4) = performs bitwise AND to clear bits 4:7,
    // leaving only bits 0:3 which are the rev ID bits.
    return (temp & (~(0x1 << 4)));
}

// Attempts to read register containing chip ID and rev ID.
bool si7210::checkGood()
{
    uint8_t temp;
    readRegister(REG_0XC0, &temp);

    if (temp == 0x14)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool si7210::sleep()
{
    uint8_t temp;
    readRegister(REG_0XC9, &temp);
    temp &= 0xFEU; // Clear sltimena
    writeRegister(REG_0XC9, temp);
    readRegister(REG_0XC4, &temp);
    temp = (temp & 0xF8U) | 0x01; // clear STOP and set SLEEP
    return writeRegister(REG_0XC4, temp);
}

bool si7210::wakeup()
{
    // Wake
    uint8_t _reg = 0xC0;
    return i2c->write(devAddr8Bit, (const char *)_reg, 1, false) == 0;

    // Reinitialize based on saved private settings
    init();
}

// B = (256*dspsigm[6:0]+dspsigl[7:0]-16384)*(0.00125 or 0.0125)
// 1 LSB = 0.00125mT (+-20.47mT scale) or 1 LSB = 0.0125mT (+-204.7mT)
int si7210::getFieldStrength()
{
    uint8_t temp;

    uint8_t dspsigm;
    readRegister(REG_DSPSIGM, &dspsigm);
    uint8_t dspsigl;
    readRegister(REG_DSPSIGL, &dspsigl);
    int fieldStrength;

    // (dspsigm & 0x7FU) = clears the MSB bit (7th bit) which is the "fresh" bit
    // which is not part of the measurement; it indicates register was updated.
    fieldStrength = ((256 * (dspsigm & 0x7FU)) + dspsigl) - 16384;

    switch (range)
    {
    case si7210_range_t::RANGE_20mT:
        return (fieldStrength / 4) + fieldStrength; // fieldStrength * 1.25
    case si7210_range_t::RANGE_200mT:
        return (fieldStrength * 12) + (fieldStrength / 2); // fieldStrength * 12.5
    default:
        return 0;
    }
}

bool si7210::setMode(si7210_mode_t m)
{
    switch (mode)
    {
    case si7210_mode_t::CONST_CONVERSION:
        // Set slFast = 1 and slTime = 0 to override the idle counter. This
        // actually means zero idle time for running in continous conversion mode.
        // Set slTimeena = 0 for IDLE mode.

        // Set slFast = 1 and slTimeena = 0
        uint8_t temp;
        readRegister(REG_0XC9, &temp);
        temp = temp | (1 << 1); // Set bit 1
        temp = temp & 0xFE;
        writeRegister(REG_0XC9, temp);

        // Set slTime = 0
        readRegister(REG_0XC8, &temp);
        temp = 0x0;
        writeRegister(REG_0XC8, temp);

        // Start measurement by
        // Clear STOP and SLEEP bits
        readRegister(REG_0XC4, &temp);
        temp = (temp & 0xFC); // Start measurement
        writeRegister(REG_0XC4, temp);

        return true;
    case si7210_mode_t::ONEBURST:
        return false;
    default:
        return false;
    }
}

bool si7210::setRange(si7210_range_t r, si7210_magnet_t mag)
{
    uint8_t temp;

    // 20mT scale and no magnetic temp. compesnation
    if (r == si7210_range_t::RANGE_20mT && mag == si7210_magnet_t::NONE)
    {
        writeRegister(REG_OTP_ADDR, 0x21U);
        writeRegister(REG_OTP_CTRL, OTP_READ_EN_MASK);
        readRegister(REG_OTP_DATA, &temp);
        writeRegister(REG_A0, temp);
        writeRegister(REG_OTP_ADDR, 0x22U);
        writeRegister(REG_OTP_CTRL, OTP_READ_EN_MASK);
        readRegister(REG_OTP_DATA, &temp);
        writeRegister(REG_A1, temp);
        writeRegister(REG_OTP_ADDR, 0x23U);
        writeRegister(REG_OTP_CTRL, OTP_READ_EN_MASK);
        readRegister(REG_OTP_DATA, &temp);
        writeRegister(REG_A2, temp);
        writeRegister(REG_OTP_ADDR, 0x24U);
        writeRegister(REG_OTP_CTRL, OTP_READ_EN_MASK);
        readRegister(REG_OTP_DATA, &temp);
        writeRegister(REG_A3, temp);
        writeRegister(REG_OTP_ADDR, 0x25U);
        writeRegister(REG_OTP_CTRL, OTP_READ_EN_MASK);
        readRegister(REG_OTP_DATA, &temp);
        writeRegister(REG_A4, temp);
        writeRegister(REG_OTP_ADDR, 0x26U);
        writeRegister(REG_OTP_CTRL, OTP_READ_EN_MASK);
        readRegister(REG_OTP_DATA, &temp);
        return writeRegister(REG_A5, temp);
    }

    // 200mT scale and no magnet temp. compensation
    else if (r == si7210_range_t::RANGE_200mT && mag == si7210_magnet_t::NONE)
    {
        writeRegister(REG_OTP_ADDR, 0x27U);
        writeRegister(REG_OTP_CTRL, OTP_READ_EN_MASK);
        readRegister(REG_OTP_DATA, &temp);
        writeRegister(REG_A0, temp);
        writeRegister(REG_OTP_ADDR, 0x28U);
        writeRegister(REG_OTP_CTRL, OTP_READ_EN_MASK);
        readRegister(REG_OTP_DATA, &temp);
        writeRegister(REG_A1, temp);
        writeRegister(REG_OTP_ADDR, 0x29U);
        writeRegister(REG_OTP_CTRL, OTP_READ_EN_MASK);
        readRegister(REG_OTP_DATA, &temp);
        writeRegister(REG_A2, temp);
        writeRegister(REG_OTP_ADDR, 0x2AU);
        writeRegister(REG_OTP_CTRL, OTP_READ_EN_MASK);
        readRegister(REG_OTP_DATA, &temp);
        writeRegister(REG_A3, temp);
        writeRegister(REG_OTP_ADDR, 0x2BU);
        writeRegister(REG_OTP_CTRL, OTP_READ_EN_MASK);
        readRegister(REG_OTP_DATA, &temp);
        writeRegister(REG_A4, temp);
        writeRegister(REG_OTP_ADDR, 0x2CU);
        writeRegister(REG_OTP_CTRL, OTP_READ_EN_MASK);
        readRegister(REG_OTP_DATA, &temp);
        return writeRegister(REG_A5, temp);
    }

    // 20mT scale and neodymium temp. comp.
    else if ((r == si7210_range_t::RANGE_20mT && mag == si7210_magnet_t::NEODYMIUM))
    {
        writeRegister(REG_OTP_ADDR, 0x2DU);
        writeRegister(REG_OTP_CTRL, OTP_READ_EN_MASK);
        readRegister(REG_OTP_DATA, &temp);
        writeRegister(REG_A0, temp);
        writeRegister(REG_OTP_ADDR, 0x2EU);
        writeRegister(REG_OTP_CTRL, OTP_READ_EN_MASK);
        readRegister(REG_OTP_DATA, &temp);
        writeRegister(REG_A1, temp);
        writeRegister(REG_OTP_ADDR, 0x2FU);
        writeRegister(REG_OTP_CTRL, OTP_READ_EN_MASK);
        readRegister(REG_OTP_DATA, &temp);
        writeRegister(REG_A2, temp);
        writeRegister(REG_OTP_ADDR, 0x30U);
        writeRegister(REG_OTP_CTRL, OTP_READ_EN_MASK);
        readRegister(REG_OTP_DATA, &temp);
        writeRegister(REG_A3, temp);
        writeRegister(REG_OTP_ADDR, 0x31U);
        writeRegister(REG_OTP_CTRL, OTP_READ_EN_MASK);
        readRegister(REG_OTP_DATA, &temp);
        writeRegister(REG_A4, temp);
        writeRegister(REG_OTP_ADDR, 0x32U);
        writeRegister(REG_OTP_CTRL, OTP_READ_EN_MASK);
        readRegister(REG_OTP_DATA, &temp);
        return writeRegister(REG_A5, temp);
    }

    // 200mT scale and neodymium temp. comp.
    else if ((r == si7210_range_t::RANGE_200mT && mag == si7210_magnet_t::NEODYMIUM))
    {
        writeRegister(REG_OTP_ADDR, 0x33U);
        writeRegister(REG_OTP_CTRL, OTP_READ_EN_MASK);
        readRegister(REG_OTP_DATA, &temp);
        writeRegister(REG_A0, temp);
        writeRegister(REG_OTP_ADDR, 0x34U);
        writeRegister(REG_OTP_CTRL, OTP_READ_EN_MASK);
        readRegister(REG_OTP_DATA, &temp);
        writeRegister(REG_A1, temp);
        writeRegister(REG_OTP_ADDR, 0x35U);
        writeRegister(REG_OTP_CTRL, OTP_READ_EN_MASK);
        readRegister(REG_OTP_DATA, &temp);
        writeRegister(REG_A2, temp);
        writeRegister(REG_OTP_ADDR, 0x36U);
        writeRegister(REG_OTP_CTRL, OTP_READ_EN_MASK);
        readRegister(REG_OTP_DATA, &temp);
        writeRegister(REG_A3, temp);
        writeRegister(REG_OTP_ADDR, 0x37U);
        writeRegister(REG_OTP_CTRL, OTP_READ_EN_MASK);
        readRegister(REG_OTP_DATA, &temp);
        writeRegister(REG_A4, temp);
        writeRegister(REG_OTP_ADDR, 0x38U);
        writeRegister(REG_OTP_CTRL, OTP_READ_EN_MASK);
        readRegister(REG_OTP_DATA, &temp);
        return writeRegister(REG_A5, temp);
    }

    // 20mT scale and ceramic temp. comp.
    else if ((r == si7210_range_t::RANGE_20mT && mag == si7210_magnet_t::CERAMIC))
    {
        writeRegister(REG_OTP_ADDR, 0x39U);
        writeRegister(REG_OTP_CTRL, OTP_READ_EN_MASK);
        readRegister(REG_OTP_DATA, &temp);
        writeRegister(REG_A0, temp);
        writeRegister(REG_OTP_ADDR, 0x3AU);
        writeRegister(REG_OTP_CTRL, OTP_READ_EN_MASK);
        readRegister(REG_OTP_DATA, &temp);
        writeRegister(REG_A1, temp);
        writeRegister(REG_OTP_ADDR, 0x3BU);
        writeRegister(REG_OTP_CTRL, OTP_READ_EN_MASK);
        readRegister(REG_OTP_DATA, &temp);
        writeRegister(REG_A2, temp);
        writeRegister(REG_OTP_ADDR, 0x3CU);
        writeRegister(REG_OTP_CTRL, OTP_READ_EN_MASK);
        readRegister(REG_OTP_DATA, &temp);
        writeRegister(REG_A3, temp);
        writeRegister(REG_OTP_ADDR, 0x3DU);
        writeRegister(REG_OTP_CTRL, OTP_READ_EN_MASK);
        readRegister(REG_OTP_DATA, &temp);
        writeRegister(REG_A4, temp);
        writeRegister(REG_OTP_ADDR, 0x3EU);
        writeRegister(REG_OTP_CTRL, OTP_READ_EN_MASK);
        readRegister(REG_OTP_DATA, &temp);
        return writeRegister(REG_A5, temp);
    }

    // 200mT scale and ceramic temp. comp.
    else if ((r == si7210_range_t::RANGE_200mT && mag == si7210_magnet_t::CERAMIC))
    {
        writeRegister(REG_OTP_ADDR, 0x3FU);
        writeRegister(REG_OTP_CTRL, OTP_READ_EN_MASK);
        readRegister(REG_OTP_DATA, &temp);
        writeRegister(REG_A0, temp);
        writeRegister(REG_OTP_ADDR, 0x40U);
        writeRegister(REG_OTP_CTRL, OTP_READ_EN_MASK);
        readRegister(REG_OTP_DATA, &temp);
        writeRegister(REG_A1, temp);
        writeRegister(REG_OTP_ADDR, 0x41U);
        writeRegister(REG_OTP_CTRL, OTP_READ_EN_MASK);
        readRegister(REG_OTP_DATA, &temp);
        writeRegister(REG_A2, temp);
        writeRegister(REG_OTP_ADDR, 0x42U);
        writeRegister(REG_OTP_CTRL, OTP_READ_EN_MASK);
        readRegister(REG_OTP_DATA, &temp);
        writeRegister(REG_A3, temp);
        writeRegister(REG_OTP_ADDR, 0x43U);
        writeRegister(REG_OTP_CTRL, OTP_READ_EN_MASK);
        readRegister(REG_OTP_DATA, &temp);
        writeRegister(REG_A4, temp);
        writeRegister(REG_OTP_ADDR, 0x44U);
        writeRegister(REG_OTP_CTRL, OTP_READ_EN_MASK);
        readRegister(REG_OTP_DATA, &temp);
        return writeRegister(REG_A5, temp);
    }
    else
    {
        return false;
    }
}

vector<si7210_register_t> si7210::i2cMemDump()
{
    vector<si7210_register_t> registers;

    for (int i = 0; i < 21; i++)
    {
        registers.push_back(si7210_register_t());
    }

    // Set Addresses
    registers[0].addr = 0xC0;
    registers[1].addr = 0xC1;
    registers[2].addr = 0xC2;
    registers[3].addr = 0xC3;
    registers[4].addr = 0xC4;
    registers[5].addr = 0xC5;
    registers[6].addr = 0xC6;
    registers[7].addr = 0xC7;
    registers[8].addr = 0xC8;
    registers[9].addr = 0xC9;
    registers[10].addr = 0xCA;
    registers[11].addr = 0xCB;
    registers[12].addr = 0xCC;
    registers[13].addr = 0xCD;
    registers[14].addr = 0xCE;
    registers[15].addr = 0xCF;
    registers[16].addr = 0xD0;
    registers[17].addr = 0xE1;
    registers[18].addr = 0xE2;
    registers[19].addr = 0xE3;
    registers[20].addr = 0xE4;

    registers.shrink_to_fit();

    for (int i = 0; i <= registers.size(); i++)
    {
        readRegister(registers[i].addr, &registers[i].data);
    }

    return registers;
}

bool si7210::setFilter(Filter f)
{
    uint8_t temp = 0x0;

    switch (f.filterType)
    {
    case si7210_filters_t::NONE:
        return writeRegister(REG_0XCD, temp);

    // FIR filter # of samples to average controlled by df_bw and is 2^df_bw
    // Max setting of 12
    case si7210_filters_t::FIR:
        if (f.burstsize > 12)
        {
            temp = 0x0;
            writeRegister(REG_0XCD, temp);
            return 0;
        }

        temp = temp | DF_FIR_MASK;
        f.burstsize = f.burstsize << 1;
        temp = temp | f.burstsize;

        return writeRegister(REG_0XCD, temp);

    // IIR filter # of samples to average controlled by df_bw and is 2^df_bw
    // Max setting of 12
    case si7210_filters_t::IIR:
        if (f.burstsize > 12)
        {
            temp = 0x0;
            writeRegister(REG_0XCD, temp);
            return 0;
        }

        temp = temp | DF_IIR_MASK;
        f.burstsize = f.burstsize << 1;
        temp = temp | f.burstsize;

        return writeRegister(REG_0XCD, temp);

    default:
        return 0;
    }
}