// File: si7210.h
// Author: David Antaki
// Date: 7/11/20
// License: This software is not open source and is copyrighted by David
// Antaki.
// Contents: A custom MBED I2C driver for the Silicon Labs Si7210 digital hall
// sensor

#ifndef SI7210_H
#define SI7210_H

#include "mbed.h"
#include <vector>
// #include "Printer.h"
// #include "utility.h"

// Device Register Addresses.
// These are defined here as macros so that comments could be added to
// show what information is stored in which registers.
#define REG_0XC0 0xC0U    // revid[0:3] ; chipid[4:7]
#define REG_DSPSIGM 0xC1U // Dspsigm[0:7]
#define REG_DSPSIGL 0xC2U // Dspsigl[0:7]
#define REG_0XC3 0xC3U    // dspsigsel[0:2]
#define REG_0XC4 0xC4U
#define REG_0XC5 0xC5U
#define REG_0XC6 0xC6U
#define REG_0XC7 0xC7U
#define REG_0XC8 0xC8U
#define REG_0XC9 0xC9U
#define REG_A0 0xCAU
#define REG_A1 0xCBU
#define REG_A2 0xCCU
#define REG_0XCD 0xCDU
#define REG_A3 0xCEU
#define REG_A4 0xCFU
#define REG_A5 0xD0U
#define REG_OTP_ADDR 0xE1U // otp_addr[0:7]
#define REG_OTP_DATA 0xE2U // otp_data[0:7]
#define REG_OTP_CTRL 0xE3U // otp_read_en[1] ; otp_busy(RO)[0]
#define REG_0XE4 0xE4U     // tm_fg[0:1]

// Bit masks
#define OTP_BUSY_MASK 1
#define OTP_READ_EN_MASK 2
#define DF_FIR_MASK 0
#define DF_IIR_MASK 1

// Possible (bipolar) measurement range settings
typedef enum class si7210_range_t
{
    RANGE_20mT,
    RANGE_200mT
} si7210_range_t;

// Possible magnet types for temp compensation
typedef enum class si7210_magnet_t
{
    NONE, // Means no temp compensation
    NEODYMIUM,
    CERAMIC
} si7210_magnet_t;

typedef enum class si7210_mode_t
{
    CONST_CONVERSION, // Continous Conversion mode
    ONEBURST          // One Burst mode
} si7210_mode_t;

typedef enum class si7210_filters_t
{
    FIR,
    IIR,
    NONE // No filter
} si7210_filters_t;

struct Filter
{
    si7210_filters_t filterType = si7210_filters_t::NONE;

    // 0-7 if IIR
    // 0-12 if FIR
    int burstsize = 0;
};

// typedef struct
// {
//     si7210_filter_t filterType;
//     unsigned int burstsize : 4;

// } si7210_filter_t;

// // Defines an IIR filter
// typedef struct
// {
//     const uint8_t DF_IIR_MASK = 1;
//     unsigned int burstSize : 3;

// } si7210_iir_t;

// // // Defines a FIR filter
// typedef struct
// {
//     const uint8_t DF_FIR_MASK = 0;
//     unsigned int burstSize : 4; // Max values of 12

// } si7210_fir_t;

// template <typename T>
// struct filter
// {
//     T filter;
// };

// // Filter type
// typedef enum class filters
// {
//     si7210_fir_t FIR,
//     si7210_iir_t IIR
// } si7210_filters_t;

// A 8-bit register
typedef struct
{
    int addr;
    uint8_t data;
} si7210_register_t;

class si7210
{
public:
    // Constructor
    //
    // @param *i2cBus   The I2C MBED object that the sensor is connected to.
    //                  This is a pointer so that other I2C devices can use
    //                  the same bus.
    // @param addr  The device address. Silicon Labs gives the device
    //                      address in 7-bits (since 8th bit is R/W bit)
    si7210(I2C *i2cBus, uint8_t addr, si7210_range_t r, si7210_magnet_t mag, si7210_mode_t m, Filter f);
    ~si7210();

    void init();

    // Reads a register (1byte) from the device's read/write I2C registers.
    // This is different from the OTP (one time programmable) register
    // which can only be read from through the I2C registers.
    //
    // @param reg   8-bit register address to read from.
    // @param *returnedData Pointer to store data read from register.
    // @return      True on success. False on failure.
    bool readRegister(uint8_t reg, uint8_t *returnedData);

    // Writes a register (1byte) to the device's read/write I2C registers.
    // This is different from the OTP (one time programmable) register
    // which can only be written to through the I2C registers.
    //
    // @param reg   8-bit register address to write to.
    // @param data  8 bits to write to the register
    // @return      True on success. False on failure.
    bool writeRegister(uint8_t reg, uint8_t data);

    // @return  The sensor's chipid. This is 0x1 for all Si7210 parts.
    uint8_t getChipId();

    // @return  The sensor's revid. This is 0x4 for revision B
    uint8_t getRevId();

    // Checks if the sensor is connected and responding.
    //
    // @return  True if connected and responding, else false.
    bool checkGood();

    bool sleep();

    bool wakeup();

    // Returns the field strength in uT measured by the sensor
    //
    // @return  The measured field strength in uTs
    int getFieldStrength();

    // Sets the sensor to continuous conversion mode where the AFE (analog
    // front end) runs continuously and a new sample is taken every 8.8usec.
    //
    // @return  True if successfully set continuous conversion, else false.
    bool setMode(si7210_mode_t m);

    // Read out the I2C registers
    vector<si7210_register_t> i2cMemDump();

private:
    // The I2C bus that this sensor is attached to.
    // Pointer so that other I2C devices can use the same bus.
    // Not a reference (&) b/c references cannot be reassigned after
    // initialization, but pointers can be reassigned.
    I2C *i2c;

    // The sensor's 7 bit device address.
    uint8_t devAddr7Bit;

    // The sensor's 8 bit device address
    uint8_t devAddr8Bit;

    // The set range/scale for the sensor.
    // Either 20mT or 200mT
    si7210_range_t range;

    // Magnet type
    si7210_magnet_t magnet;

    // The mode the sensor is currently set to.
    si7210_mode_t mode;

    // Filter
    Filter filter;

    // Sets the range of the sensor
    // RANGE_20mT = +-20mT
    // RANGE_200mT = +-200mT
    // With temp compensation based on magnet type
    bool setRange(si7210_range_t _range, si7210_magnet_t mag);

    // Sets the filter type
    bool setFilter(Filter f);
};

#endif