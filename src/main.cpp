// File: main.cpp
// Author: David Antaki
// Date: 4/16/2020
// License: This software is not open source and is copyrighted by David
// Antaki and Sous Bois International, Inc.
// Contents: Program entry.

#ifndef MAIN_H
#define MAIN_H

#include "mbed.h"
#include <bitset>
#include "si7210.h"
#include "utility.h"
#include "Printer.h"
#include <vector>

// Code insertion MACOROs
// If don't want code insertion, change #define to #undef
#define PRINT_TEST_INPUT_OUTPUT
#undef UNIT_TEST
#undef ACCURACY_TESTING
#define PRECISION_TESTING

void printRegisters(vector<si7210_register_t> _registers)
{
  for (int i = 0; i < _registers.size(); i++)
  {
    Printer::pc.printf("0x%X: %s\n", _registers[i].addr, utility::hexToBinaryStr(_registers[i].data).c_str());
  }
}

void test_getChipId(si7210 hall)
{
  uint8_t temp = hall.getChipId();

#ifdef PRINT_TEST_INPUT_OUTPUT
  std::bitset<8> binary(temp);
  std::string binaryStr = binary.to_string<char, std::string::traits_type, std::string::allocator_type>();
  Printer::pc.printf("%s:\n", __FUNCTION__);
  Printer::pc.printf("\tEXPECTED:\tHex: 0x1\tBinary: 00000001\n");
  Printer::pc.printf("\tACTUAL:\t\tHex: 0x%x\tBinary: %s\n", temp, binaryStr.c_str());
#endif

  Printer::pc.printf("\t%s Result: %d\n", __FUNCTION__, temp == 0x1);
}

void test_getRevId(si7210 hall)
{
  uint8_t temp = hall.getRevId();

#ifdef PRINT_TEST_INPUT_OUTPUT
  std::bitset<8> binary(temp);
  std::string binaryStr = binary.to_string<char, std::string::traits_type, std::string::allocator_type>();
  Printer::pc.printf("%s:\n", __FUNCTION__);
  Printer::pc.printf("\tEXPECTED:\tHex: 0x4\tBinary: 00000100\n");
  Printer::pc.printf("\tACTUAL:\t\tHex: 0x%x\tBinary: %s\n", temp, binaryStr.c_str());
#endif

  Printer::pc.printf("\t%s Result: %d\n", __FUNCTION__, temp == 0x4);
}

void test_checkGood(si7210 hall)
{
  bool temp = hall.checkGood();

#ifdef PRINT_TEST_INPUT_OUTPUT
  Printer::pc.printf("%s:\n", __FUNCTION__);
  Printer::pc.printf("\tEXPECTED: 1\n");
  Printer::pc.printf("\tACTUAL: %d\n", temp);
#endif

  Printer::pc.printf("\t%s Result: %d\n", __FUNCTION__, temp);
}

// void test_setContinousConversion(si7210 hall)
// {
//   // TEST SET ON

//   // Read the register before writing it
//   uint8_t reg_0XC9_before;
//   hall.readRegister(REG_0XC9, &reg_0XC9_before);
//   uint8_t reg_0XC8_before;
//   hall.readRegister(REG_0XC8, &reg_0XC8_before);

//   bool success = hall.setContinousConversion(ON);

//   // Read the register after writing it
//   uint8_t reg_0XC9_after;
//   hall.readRegister(REG_0XC9, &reg_0XC9_after);
//   uint8_t reg_0XC8_after;
//   hall.readRegister(REG_0XC8, &reg_0XC8_after);

// #ifdef PRINT_TEST_INPUT_OUTPUT
//   Printer::pc.printf("%s ON:\n", __FUNCTION__);
//   Printer::pc.printf("\t0XC9 REGISTER BEFORE:\tHex: 0x%x\tBinary: %s\n", reg_0XC9_before, utility::hexToBinaryStr(reg_0XC9_before).c_str());
//   Printer::pc.printf("\t0XC9 REGISTER AFTER:\tHex: 0x%x\tBinary: %s\n", reg_0XC9_after, utility::hexToBinaryStr(reg_0XC9_after).c_str());
//   Printer::pc.printf("\t0XC9 EXPECTED:\t\tHex: NA\t\tBinary: XXXXXX10\n\n");

//   Printer::pc.printf("\t0XC8 REGISTER BEFORE:\tHex: 0x%x\tBinary: %s\n", reg_0XC8_before, utility::hexToBinaryStr(reg_0XC8_before).c_str());
//   Printer::pc.printf("\t0XC8 REGISTER AFTER:\tHex: 0x%x\tBinary: %s\n", reg_0XC8_after, utility::hexToBinaryStr(reg_0XC8_after).c_str());
//   Printer::pc.printf("\t0XC8 EXPECTED:\t\tHex: NA\t\tBinary: 00000000\n");
// #endif

//   // Verify result
//   bool result = 0;
//   if ((reg_0XC9_after & 0x03) &&
//       reg_0XC8_after == 0)
//     result = 1;

//   Printer::pc.printf("\t%s ON Result: %d\n", __FUNCTION__, result);

//   // -------------------------------------------------------------------------
//   // TEST SET OFF

//   // Read the register before writing it
//   hall.readRegister(REG_0XC9, &reg_0XC9_before);
//   hall.readRegister(REG_0XC8, &reg_0XC8_before);

//   success = hall.setContinousConversion(OFF);

//   // Read the register after writing it
//   hall.readRegister(REG_0XC9, &reg_0XC9_after);
//   hall.readRegister(REG_0XC8, &reg_0XC8_after);

// #ifdef PRINT_TEST_INPUT_OUTPUT
//   Printer::pc.printf("%s OFF:\n", __FUNCTION__);
//   Printer::pc.printf("\t0XC9 REGISTER BEFORE:\tHex: 0x%x\tBinary: %s\n", reg_0XC9_before, utility::hexToBinaryStr(reg_0XC9_before).c_str());
//   Printer::pc.printf("\t0XC9 REGISTER AFTER:\tHex: 0x%x\tBinary: %s\n\n", reg_0XC9_after, utility::hexToBinaryStr(reg_0XC9_after).c_str());

//   Printer::pc.printf("\t0XC8 REGISTER BEFORE:\tHex: 0x%x\tBinary: %s\n", reg_0XC8_before, utility::hexToBinaryStr(reg_0XC8_before).c_str());
//   Printer::pc.printf("\t0XC8 REGISTER AFTER:\tHex: 0x%x\tBinary: %s\n", reg_0XC8_after, utility::hexToBinaryStr(reg_0XC8_after).c_str());
// #endif

//   Printer::pc.printf("\t%s OFF Result: %d\n", __FUNCTION__, reg_0XC9_after == ((reg_0XC9_before & 0xF8) | 0x2));
// }

void test_i2cMemReadout(si7210 hall)
{
  printRegisters(hall.i2cMemDump());
}

#ifdef UNIT_TEST

// Program entry point.
// No arguments should be passed.
int main(int argc, char *argv[])
{

  // The device address
  uint8_t devAddr7Bit = 0x31U;

  // Create the I2C bus
  PinName sda = PA_10;
  PinName scl = PA_9;
  I2C i2c(sda, scl);

  // Create an si7210 object
  si7210 hall(&i2c, devAddr7Bit);

  thread_sleep_for(2000);

  while (1)
  {
    thread_sleep_for(5000);
    Printer::pc.printf("------------------------------------------------------------------------\n");

    // Run tests
    test_getChipId(hall);
    test_getRevId(hall);
    test_checkGood(hall);
    test_setContinousConversion(hall);
    test_i2cMemReadout(hall);

    Printer::pc.printf("------------------------------------------------------------------------\n");
  }
}
#endif //UNIT_TEST

// testing the accuracy aka linearity aka how close the hall sensor readings
// are to what they're actually suppose to be
#ifdef ACCURACY_TESTING

int main(int argc, char *argv[])
{

  // The device address
  uint8_t devAddr7Bit = 0x31U;

  // Create the I2C bus
  PinName sda = PA_10;
  PinName scl = PA_9;
  I2C i2c(sda, scl);

  // Set I2C bus freqeuncy to 1MHz
  i2c.frequency(1000000);

  // Create an si7210 object
  si7210 hall(&i2c, devAddr7Bit);

  int tempFieldStrength;

  hall.setContinousConversion();

  hall.setRange(RANGE_200mT);

  thread_sleep_for(2000);

  while (1)
  {
    tempFieldStrength = hall.getFieldStrength();

    thread_sleep_for(50);
    // Printer::pc.printf("Field Strength (uT) %i\t", tempFieldStrength);
    Printer::pc.printf("Field Strength (uT) %i\n", hall.getFieldStrength());
    thread_sleep_for(50);

    uint8_t temp;
    hall.readRegister(REG_0XC4, &temp);
    // Printer::pc.printf("\t0XC4 REGISTER:\tHex: 0x%x\tBinary: %s\n", temp, utility::hexToBinaryStr(temp).c_str());

    // Printer::pc.printf("%s", utility::hexToBinaryStr(hall.setContinousConversion()).c_str());
  }
}

#endif //ACCURACY_TESTING

// testing precision aka noise
// testing sample time too
#ifdef PRECISION_TESTING

// settings
#define TEST_TIME 99999

int main(int argc, char *argv[])
{

  // Device address
  uint8_t devAddr7Bit = 0x31U;

  // I2C bus
  PinName sda = PA_10;
  PinName scl = PA_9;
  I2C i2c(sda, scl);
  i2c.frequency(1000000);

  // Filter
  Filter filter;
  filter.filterType = si7210_filters_t::FIR;
  filter.burstsize = 12;

  // si7210 object
  si7210 hall(&i2c, devAddr7Bit, si7210_range_t::RANGE_20mT, si7210_magnet_t::NEODYMIUM, si7210_mode_t::CONST_CONVERSION, filter);

  int tempFieldStrength;

  Timer time;
  time.start();

  // Timer for timing sample time
  Timer sampleTime;

  thread_sleep_for(2000);

  time.reset();

  while (1)
  {
    sampleTime.start();
    sampleTime.reset();

    tempFieldStrength = hall.getFieldStrength();

    sampleTime.stop();
    thread_sleep_for(3);

    Printer::pc.printf("Time (ms): %i\t", time.read_ms());
    Printer::pc.printf("SampleTime (us): %i\t", sampleTime.read_us());
    Printer::pc.printf("Field Strength (uT): %i\t", tempFieldStrength);

    uint8_t tempReg;
    hall.readRegister(REG_0XCD, &tempReg);
    Printer::pc.printf("0XCD: %s\n", utility::hexToBinaryStr(tempReg).c_str());

    thread_sleep_for(3);

    if (time.read() > TEST_TIME)
      break;

    // printRegisters(hall.i2cMemDump());
  }
}

#endif //PRECISION_TESTING

#endif //MAIN_H