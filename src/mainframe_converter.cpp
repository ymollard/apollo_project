#include "mainframe_converter.h"


u_int64_t MainframeConverter::to_8bits_ibm_7044(u_int64_t input) {
    // Converts a 6bit padded long int (e.g. XXXXXX00XXXXXX00XXXXXXX) into host long int (XXXXXXXXXXXXXXXXXXX)
    return (input & 0x3F) | ((input & 0x3F00) >> 2)  | ((input & 0x3F0000) >> 4) |
                          ((input & 0x3F000000) >> 6)  | ((input & 0x3F00000000) >> 8) |
                          ((input & 0x3F0000000000) >> 10);
}

u_int64_t MainframeConverter::read_int_ibm_7044(std::ifstream &f, bool debug) {
    // Read a LSB-first int of 6 Bytes and return the corresponding encoding for the host
    u_int64_t value = 0;
    f.read((char *)&value, ENCODED_WORD_LENGTH_IBM_7044);
    return to_int_ibm_7044(value, debug);
}

u_int64_t MainframeConverter::to_int_ibm_7044(u_int64_t value, bool debug) {
    if(debug) std::cout << "Raw read int: 0x" << std::hex << value;
    value = be64toh(value);
    // Now shift the MSB-first value to restore the 36bit int
    value >>= 8*(sizeof(u_int64_t)-ENCODED_WORD_LENGTH_IBM_7044);
    value = to_8bits_ibm_7044(value);
    if(debug) std::cout << ", formatted int: 0x" << std::hex << value << std::endl;
    return value;
}

double MainframeConverter::read_float_ibm_7044(std::ifstream &f, bool debug) {
    // Read a LSB-first float of 6 Bytes and return the IEEE-754 representation
    u_int64_t value = 0;
    f.read((char *)&value, ENCODED_WORD_LENGTH_IBM_7044);
    return to_float_ibm_7044(value, debug);
}

double MainframeConverter::to_float_ibm_7044(u_int64_t value, bool debug) {
    if(debug) std::cout << "Raw read float: 0x" << std::hex << value;
    value = be64toh(value);
    // Now shift the MSB-first value to restore the 36bit float
    value >>= 8*(sizeof(u_int64_t)-ENCODED_WORD_LENGTH_IBM_7044);
    value = to_8bits_ibm_7044(value);
    if(debug) std::cout << ", formatted float: 0x" << std::hex << value << std::endl;

    // Now convert the IBM 7044/7090/7094 float (36-bit) into host float
    // Hint: From http://nssdc.gsfc.nasa.gov/nssdc/formats/IBM7044_7090_7094.htm
    // EXPONENT (E): To the base 2, with a bias in the single precision floating point of 128 decimal (200 octal), and in the double precision floating point of 1024 decimal (2000 octal).
    // FRACTION: Has no hidden bit.
    // The value of the number N is N = F x 2^(E-128) (Single precision), or N = F x 2^(E-1024) (double precision) where F is a binary fraction such that 0 <= |F| <1

    double fraction = 0, abs;
    unsigned int exponent = (value >> 27) & 0xFF;

    for(int i=0; i<27; ++i) {
        if(((value & 0x7FFFFFF) >> i) & 1) {
            fraction += pow(2, i-27.0);
            if(debug) std::cout << "Adding 2^" << std::dec << i-27 << std::endl;
            }
    }
    abs = fraction*pow(2, exponent-128);
    if(debug) std::cout << std::fixed << fraction << "*2^(" << exponent << "-128) = "<< abs << std::endl;
    return ((value >> 35) & 0x1)? -abs:abs;
}

double MainframeConverter::read_float_ibm_360(std::ifstream &f, bool debug) {
    u_int32_t value = 0;
    f.read((char *)&value, ENCODED_WORD_LENGTH_IBM_360);
    return to_float_ibm_360(value, debug);
}

double MainframeConverter::to_float_ibm_360(u_int32_t value, bool debug) {
    /* Convert a 32-bits float value coming from IBM 360 into an IEEE-754 standard representation */
    if(debug) std::cout << "Raw read float: 0x" << std::hex << value;
    value = be32toh(value);
    if(debug) std::cout << ", formatted float: 0x" << std::hex << value << std::endl;

    // Now convert the IBM 360/37/3081 float (32-bit) into host float
    // Hint: From http://nssdc.gsfc.nasa.gov/nssdc/formats/IBM_32-Bit.html
    // IBM floating point numbers are represented by one bit for the sign (S), 7 bits for the exponent, and 24 bits for the fraction.
    // The exponent is to the base 16 (not 2), and has a bias of 64.

    double fraction = 0, abs;
    unsigned int exponent = (value >> 24) & 0x7F;

    for(int i=0; i<24; ++i) {
        if(((value & 0x7FFFFFF) >> i) & 1) {
            fraction += pow(2, i-24.0);
            if(debug) std::cout << "Adding 2^" << std::dec << i-24 << std::endl;
            }
    }
    abs = fraction*pow(16, exponent-64);
    if(debug) std::cout << std::fixed << fraction << "*16^(" << exponent << "-64) = "<< abs << std::endl;
    return ((value >> 35) & 0x1)? -abs:abs;
}

unsigned int MainframeConverter::read_int_ibm_360(std::ifstream &f, bool debug) {
    u_int32_t value = 0;
    f.read((char *)&value, ENCODED_WORD_LENGTH_IBM_360);
    return value;
}
