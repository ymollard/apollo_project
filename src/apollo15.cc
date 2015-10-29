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

/* other includes */
#include "apollo15.hpp"
#include "jpleph.h"

using namespace Info;
using namespace std;

#define TRIM 			0 /* Bytes to remove to reach the first record in the file */
#define ENCODED_WORD_LENGTH     6 /* Length in Bytes of a word as encoded in the file, int or float */
#define OUTPUT_PATH "./fits/"

#define STR_VALUE(arg) #arg
#define CHECK_END() if(feof(f)) { cout << "File ended" << endl; f.close(); dataSetServer->close(set); exit(EXIT_SUCCESS); }
#define ASSERT(x) if(!(x)) { cout << "At 0x" << hex << f.tellg() << dec << " assert failed: " << STR_VALUE(x) << endl; f.close(); exit(EXIT_SUCCESS); } // Allows to close all files properly when a new spectra is not complete (eof)

unsigned long int read_int(ifstream &f, bool debug=false) {
    // Read a LSB-first int of 6 Bytes and return the corresponding encoding for the host
    unsigned long int value = 0;
    f.read((char *)&value, ENCODED_WORD_LENGTH);
    if(debug) cout << "Raw read int: 0x" << hex << value;
    value = be64toh(value);
    // Now shift the MSB-first value to restore the 36bit int
    value >>= 8*(sizeof(unsigned long int)-ENCODED_WORD_LENGTH);
    if(debug) cout << ", formatted int: 0x" << hex << value << endl;
    return value;
}

unsigned long int to_8bits(unsigned long int input) {
    // Converts a 6bit padded long int (e.g. XXXXXX00XXXXXX00XXXXXXX) into host long int (XXXXXXXXXXXXXXXXXXX)
    return (input & 0x3F) | ((input & 0x3F00) >> 2)  | ((input & 0x3F0000) >> 4) |
                          ((input & 0x3F000000) >> 6)  | ((input & 0x3F00000000) >> 8);
}

double read_float(ifstream &f, bool debug=false) {
    // Read a LSB-first float of 6 Bytes and return the IEEE-754 representation
    unsigned long int value = 0;
    f.read((char *)&value, ENCODED_WORD_LENGTH);
    if(debug) cout << "Raw read float: 0x" << hex << value;
    value = be64toh(value);
    // Now shift the MSB-first value to restore the 36bit float
    value >>= 8*(sizeof(unsigned long int)-ENCODED_WORD_LENGTH);
    if(debug) cout << ", formatted float: 0x" << hex << value << endl;
    return 0;
}

void read_binary(ifstream &f)
{
    unsigned long int int_val;
    double float_val;

    f.ignore(TRIM);
    while(!f.eof()) {

        //**** RECORD 1: 32 words, spacecraft trajectory parameters, all floats
        f.ignore(2); // remove 0xC0 = 192 before record 1
        cout << "At 0x" << hex << f.tellg() << ":" << endl;

        for(int i=0; i<32 && !f.eof(); ++i) {
            cout << "Trajectory parameter " << dec << i+1 << " ";
            float_val = read_float(f, true);
            if(i==0) {
                cout << "Record 1, GMT: " << float_val << endl;
            }
            //cout << "Trajectory parameter " << dec << i+1 << ": " << float_val << endl;
        }

        //**** RECORD 2: 13 words, x-ray and gamma-ray common data, all ints except words 2 3 and 13 (floats)
        f.ignore(2); // remove 0x4E = 78 before record 2

        for(int i=0; i<13 && !f.eof(); ++i) {
            if(i==1 || i==2 || i==12) {  // These are floats
                float_val = read_float(f);
            }
            else {  // The rest is ints
                int_val = read_int(f);
            }

            if(i==0) {
                cout << "Record 2, GMT: " << int_val << endl;
            }
        }

        //**** RECORD 3: 13 words, housekeeping data, merged ints/floats
        f.ignore(2); // remove 0x4E = 78 before record 3

        for(int i=0; i<13 && !f.eof(); ++i) {
            if(i==1 || i==2 || i==3 || i==7 || i==8 || i==9) {  // These are floats
                float_val = read_float(f);
            }
            else {  // The rest is ints
                int_val = read_int(f);
                //cout << "Record 3 parameter " << dec << i+1 << ": " << int_val << endl;
            }
        }

        //**** RECORD 4: 513 words, GMT followed by gamma-ray counts, all integers
        f.ignore(2); // remove 0xC06 = 3078 before record 4

        int_val = read_int(f);
        cout << "Record 4, GMT: " << dec << to_8bits(int_val) << endl;

        for(int i=0; i<512 && !f.eof(); ++i) {
            int_val = read_int(f);

            //if(value>0) cout << i << ": " << value << endl;
        }
    }
}

int main(int argc, char **argv) {
    assert(sizeof(unsigned long int)>ENCODED_WORD_LENGTH); // Only 64 bit machines may handle enough space in only one value
    assert(sizeof(float)==4);

    errHandler.name(argv[0]);

    openParameters("apollo15");
    gParameters->parseCommandLine(argc,argv);
    gParameters->checkMandatoryParameters();

    //int i;
    //bool calib = booleanParameter("calibration"); /* If true, output calibration data instead of normal data */
    //bool sumcontrol = booleanParameter("sumcontrol"); /* If true, put a negative flag to measurements failing at sum check */
    //int observation = intParameter("observation");
    //string filename = OUTPUT_PATH + cut_name(observation) + "_" + (calib? string("calibration_"):string("")) + string("apollo15.fits"); /* output file name */
    string input = stringParameter("inputfile"); /* input filename */

    ifstream f(input.c_str(), ios::in | ios::binary); // Input should be the file DR005893.F01 without Record sizes (use remove_blocks.cc to preprocess DR005893.F01)
    if(!f.is_open())  { perror("unable to open file"); exit(EXIT_FAILURE); }

    read_binary(f);

    cout << "Conversion ended, output file: " << /*filename <<*/ endl;
    return EXIT_SUCCESS;
}
