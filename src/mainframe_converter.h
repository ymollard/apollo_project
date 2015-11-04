#ifndef MAINFRAME_CONVERTER_H
#define MAINFRAME_CONVERTER_H
#include <arpa/inet.h>
#include <types.h>
#include <math.h>
#include <iostream>
#include <fstream>

#define ENCODED_WORD_LENGTH_IBM_7044     6 /* Length in Bytes of a word as encoded in the file, int or float */
#define ENCODED_WORD_LENGTH_IBM_360      4 /* Length in Bytes of a word as encoded in the file, int or float */

class MainframeConverter {
    public:
        /****** IBM  IBM 7044/7090/7094 (36-bit words with padding every 6 bit) ******/
        static u_int64_t to_8bits_ibm_7044(u_int64_t input);
        // Converts a 6bit padded long int (e.g. XXXXXX00XXXXXX00XXXXXXX) into host long int (XXXXXXXXXXXXXXXXXXX)

        static u_int64_t read_int_ibm_7044(std::ifstream &f, bool debug=false);
        // Read a LSB-first int of 6 Bytes and return the corresponding encoding for the host

        static u_int64_t to_int_ibm_7044(u_int64_t value, bool debug=false);

        static double read_float_ibm_7044(std::ifstream &f, bool debug=false);
        // Read a LSB-first float of 6 Bytes and return the IEEE-754 representation

        static double to_float_ibm_7044(u_int64_t value, bool debug=false);


        /****** IBM 360 (32-bit words) ******/
        static double read_float_ibm_360(std::ifstream &f, bool debug=false);
        static double to_float_ibm_360(u_int32_t value, bool debug);
        static unsigned int read_int_ibm_360(std::ifstream &f, bool debug=false);

        /****** Other ******/
        u_int16_t read_short_16b(std::ifstream &f);
};

#endif // MAINFRAME_CONVERTER_H
