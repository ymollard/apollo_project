/*
 * apollo15.cc
 *
 *  Created on: Oct 26, 2015
 *      Author: ymollard
 */

/* system includes */
#include <arpa/inet.h>
#include <math.h>
#include <assert.h>
#include <iostream>
#include <fstream>

/* SAS includes */
#include <Dal.h>
#include <param/src/Parameter.h>
#include <param/src/param.h>
#include <utils/src/STime.h>
#include "DalRegister.h"

using namespace Info;
using namespace std;

#define TRIM 			0 /* Bytes to remove to reach the first record in the file */
#define ENCODED_WORD_LENGTH     6 /* Length in Bytes of a word as encoded in the file, int or float */

unsigned long int to_8bits(unsigned long int input) {
    // Converts a 6bit padded long int (e.g. XXXXXX00XXXXXX00XXXXXXX) into host long int (XXXXXXXXXXXXXXXXXXX)
    return (input & 0x3F) | ((input & 0x3F00) >> 2)  | ((input & 0x3F0000) >> 4) |
                          ((input & 0x3F000000) >> 6)  | ((input & 0x3F00000000) >> 8) |
                          ((input & 0x3F0000000000) >> 10);
}

unsigned long int read_int(ifstream &f, bool debug=false) {
    // Read a LSB-first int of 6 Bytes and return the corresponding encoding for the host
    unsigned long int value = 0;
    f.read((char *)&value, ENCODED_WORD_LENGTH);
    if(debug) cout << "Raw read int: 0x" << hex << value;
    value = be64toh(value);
    // Now shift the MSB-first value to restore the 36bit int
    value >>= 8*(sizeof(unsigned long int)-ENCODED_WORD_LENGTH);
    value = to_8bits(value);
    if(debug) cout << ", formatted int: 0x" << hex << value << endl;
    return value;
}

double read_float(ifstream &f, bool debug=false) {
    // Read a LSB-first float of 6 Bytes and return the IEEE-754 representation
    unsigned long int value = 0;
    f.read((char *)&value, ENCODED_WORD_LENGTH);
    if(debug) cout << "Raw read float: 0x" << hex << value;
    value = be64toh(value);
    // Now shift the MSB-first value to restore the 36bit float
    value >>= 8*(sizeof(unsigned long int)-ENCODED_WORD_LENGTH);
    value = to_8bits(value);
    if(debug) cout << ", formatted float: 0x" << hex << value << endl;

    // Now convert the IBM 7044/7090/7094 float (36-bit) into host float
    // Hint: From http://nssdc.gsfc.nasa.gov/nssdc/formats/IBM7044_7090_7094.htm
    // EXPONENT (E): To the base 2, with a bias in the single precision floating point of 128 decimal (200 octal), and in the double precision floating point of 1024 decimal (2000 octal).
    // FRACTION: Has no hidden bit.
    // The value of the number N is N = F x 2^(E-128) (Single precision), or N = F x 2^(E-1024) (double precision) where F is a binary fraction such that 0 <= |F| <1

    double fraction = 0, abs;
    int exponent = (value >> 27) & 0xFF;

    for(int i=0; i<27; ++i) {
        if(((value & 0x7FFFFFF) >> i) & 1) {
            fraction += pow(2, i-27.0);
            if(debug) cout << "Adding 2^" << dec << i-27 << endl;
            }
    }
    abs = fraction*pow(2, exponent-128);
    if(debug) cout << fixed << fraction << "*2^(" << exponent << "-128) = "<< abs << endl;
    return ((value >> 35) & 0x1)? -abs:abs;
}

void read_binary(ifstream &f, ofstream &csv)
{
    unsigned long int int_val;
    double float_val;

    f.ignore(TRIM);
    while(!f.eof()) {

        //**** RECORD 1: 32 words, spacecraft trajectory parameters, all floats
        f.ignore(2); // remove 0xC0 = 192 before record 1
        cout << "At 0x" << hex << f.tellg() << ":" << endl;

        for(int i=0; i<32 && !f.eof(); ++i) {
            float_val = read_float(f);

            if(i==0) {
                cout << "Record 1, GMT: " << dec << fixed << float_val << endl;
            }
            //cout << "Trajectory parameter " << dec << i+1 << ": " << fixed << float_val << endl;
            if(csv.is_open()) csv << fixed << float_val << ";";
        }

        //**** RECORD 2: 13 words, x-ray and gamma-ray common data, all ints except words 2 3 and 13 (floats)
        f.ignore(2); // remove 0x4E = 78 before record 2

        for(int i=0; i<13 && !f.eof(); ++i) {
            if(i==1 || i==2 || i==12) {  // These are floats
                float_val = read_float(f);
                if(csv.is_open()) csv << fixed << float_val << ";";
            }
            else {  // The rest is ints
                int_val = read_int(f);
                if(csv.is_open()) csv << fixed << int_val << ";";
            }

            if(i==0) {
                //cout << "Record 2, GMT: " << int_val << endl;
            }
        }

        //**** RECORD 3: 13 words, housekeeping data, merged ints/floats
        f.ignore(2); // remove 0x4E = 78 before record 3

        for(int i=0; i<13 && !f.eof(); ++i) {
            if(i==1 || i==2 || i==3 || i==7 || i==8 || i==9) {  // These are floats
                float_val = read_float(f);
                //cout << "Record 3 parameter " << dec << i+1 << ": " << int_val << endl;
                if(csv.is_open()) csv << fixed << float_val << ";";
            }
            else {  // The rest is ints
                int_val = read_int(f);
                //cout << "Record 3 parameter " << dec << i+1 << ": " << int_val << endl;
                if(csv.is_open()) csv << fixed << int_val << ";";
            }
        }

        //**** RECORD 4: 513 words, GMT followed by gamma-ray counts, all integers
        f.ignore(2); // remove 0xC06 = 3078 before record 4

        int_val = read_int(f);
        //cout << "Record 4, GMT: " << dec << int_val << endl;
        if(csv.is_open()) csv << fixed << int_val << ";";

        for(int i=0; i<512 && !f.eof(); ++i) {
            int_val = read_int(f);
            if(csv.is_open()) csv << fixed << int_val << ";";
            //if(value>0) cout << i << ": " << value << endl;
        }
        if(csv.is_open()) csv << "\n";
    }
}

int main(int argc, char **argv) {
    assert(sizeof(unsigned long int)>ENCODED_WORD_LENGTH); // Only 64 bit machines may handle enough space in only one value
    assert(sizeof(float)==4);

    errHandler.name(argv[0]);

    openParameters("apollo15");
    gParameters->parseCommandLine(argc,argv);
    gParameters->checkMandatoryParameters();

    string input_file = stringParameter("inputfile"); /* input filename */

    ifstream input_binary(input_file.c_str(), ios::in | ios::binary);
    if(!input_binary.is_open())  { perror("unable to open file"); exit(EXIT_FAILURE); }

    ofstream output_csv;
    output_csv.open(input_file.append(".csv").c_str()); // Comment to disable CSV output

    read_binary(input_binary, output_csv);
    if(output_csv.is_open()) output_csv.close();

    cout << "Conversion ended" << endl;
    return EXIT_SUCCESS;
}
