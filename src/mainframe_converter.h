#ifndef MAINFRAME_CONVERTER_H
#define MAINFRAME_CONVERTER_H
#include <arpa/inet.h>
#include <types.h>
#include <math.h>
#include <iostream>
#include <fstream>

#define ENCODED_WORD_LENGTH_IBM_7044     6 /* Length in Bytes of a word as encoded in the file, int or float */

class MainframeConverter {
    public:
        static u_int64_t to_8bits_ibm_7044(u_int64_t input);
        // Converts a 6bit padded long int (e.g. XXXXXX00XXXXXX00XXXXXXX) into host long int (XXXXXXXXXXXXXXXXXXX)

        static u_int64_t read_int_ibm_7044(std::ifstream &f, bool debug=false);
        // Read a LSB-first int of 6 Bytes and return the corresponding encoding for the host

        static u_int64_t to_int_ibm_7044(u_int64_t value, bool debug=false);

        static double read_float_ibm_7044(std::ifstream &f, bool debug=false);
        // Read a LSB-first float of 6 Bytes and return the IEEE-754 representation

        static double to_float_ibm_7044(u_int64_t value, bool debug=false);
};

#endif // MAINFRAME_CONVERTER_H
