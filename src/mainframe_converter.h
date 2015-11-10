#ifndef MAINFRAME_CONVERTER_H
#define MAINFRAME_CONVERTER_H
#include <arpa/inet.h>
#include <types.h>
#include <math.h>
#include <iostream>
#include <fstream>
#include <typeinfo>

#define ENCODED_WORD_LENGTH_IBM_7044     6 /* Length in Bytes of a word as encoded in the file, int or float */
#define ENCODED_WORD_LENGTH_IBM_360      4 /* Length in Bytes of a word as encoded in the file, int or float */

#define CONSISTENCY_PASSED 0
#define CONSISTENCY_FAILED -1
#define CONSISTENCY_EOF -2

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

        template <typename word_ibm_7044>
        int check_consistency_and_align(std::ifstream &f, word_ibm_7044 value, word_ibm_7044 min_acceptable_value, word_ibm_7044 max_acceptable_value, off_t max_offset=0, bool debug=false);  // Template implementation in .tcc file


        /****** IBM 360 (32-bit words) ******/
        static double read_float_ibm_360(std::ifstream &f, bool debug=false);
        static double to_float_ibm_360(u_int32_t value, bool debug);
        static unsigned int read_int_ibm_360(std::ifstream &f, bool debug=false);

        /****** Other ******/
        u_int16_t read_short_16b(std::ifstream &f);
};

#include "mainframe_converter.tcc" // In-header implementation of templates

#endif // MAINFRAME_CONVERTER_H
