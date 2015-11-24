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

#define TYPE_INT_IBM_7044 1
#define TYPE_FLOAT_IBM_7044 2
#define TYPE_INT_IBM_360 3
#define TYPE_FLOAT_IBM_360 4

class MainframeConverter {
    protected:
        bool eof; // End of file
        bool eot; // End of tape
    public:
        /****** IBM  IBM 7044/7090/7094 (36-bit words with padding every 6 bit) ******/
        static u_int64_t to_8bits_ibm_7044(u_int64_t input);
        // Converts a 6bit padded long int (e.g. XXXXXX00XXXXXX00XXXXXXX) into host long int (XXXXXXXXXXXXXXXXXXX)

        u_int64_t read_int_ibm_7044(std::ifstream &f, bool debug=false);
        // Read a LSB-first int of 6 Bytes and return the corresponding encoding for the host

        static u_int64_t to_int_ibm_7044(u_int64_t value, bool debug=false);

        double read_float_ibm_7044(std::ifstream &f, bool debug=false);
        // Read a LSB-first float of 6 Bytes and return the IEEE-754 representation

        static double to_float_ibm_7044(u_int64_t value, bool debug=false);

        /****** IBM 360 (32-bit words) ******/
        static double read_float_ibm_360(std::ifstream &f, bool debug=false);
        static double to_float_ibm_360(u_int32_t value, bool debug);
        static unsigned int read_int_ibm_360(std::ifstream &f, bool debug=false);

        /****** Other ******/
        u_int16_t read_short_16b(std::ifstream &f);
        template <typename word>
        int check_consistency_and_align(std::ifstream &f, int word_type, word value, word min_acceptable_value, word max_acceptable_value, off_t offset_to_new_record, off_t max_offset, bool debug=false);

        MainframeConverter();
};

#include "mainframe_converter.tcc" // In-header implementation of templates

#endif // MAINFRAME_CONVERTER_H
