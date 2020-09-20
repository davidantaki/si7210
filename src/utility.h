// File: Utility.h
// Author: David Antaki
// Date: 7/30/2020
// License: This software is not open source and is copyrighted by David
// Antaki.
// Contents: For utility functions

#ifndef UTILITY_H
#define UTILITY_H

#include "mbed.h"
#include <bitset>

class utility
{
public:
    // Converts one byte of data in hex to binary format
    // @param hex   The hex formatted byte.
    // @return      A binary in string format.
    static std::string hexToBinaryStr(uint8_t _hex);
};

std::string utility::hexToBinaryStr(uint8_t _hex)
{
    std::bitset<8> binary(_hex);
    return binary.to_string<char, std::string::traits_type, std::string::allocator_type>();
}

#endif //UTILITY_H
