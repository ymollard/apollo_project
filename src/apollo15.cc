/*
 * apollo15.cc
 *
 *  Created on: Jun 29, 2012
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
//#include "Celestial.h"

/* other includes */
#include "apollo15.hpp"
#include "jpleph.h"

//#include "Celestial.cc" // TODO dirty thing

using namespace Info;
using namespace std;

#define TRIM 			0 /* Bytes to remove to reach the first record in the file */
#define WORD_LENGTH     6 /* Length in Bytes of a single word, int or float */
#define INSTRUMENT_ROLL	37.75*DEG_TO_RAD  /* instrument angle around the roll axis (x) */
#define JPLEPH_PATH 	"/home/ymollard/ctadev/lib/JPLEPH"
#define OUTPUT_PATH "./fits/"

#define STR_VALUE(arg) #arg
#define CHECK_END() if(feof(f)) { cout << "File ended" << endl; f.close(); dataSetServer->close(set); exit(EXIT_SUCCESS); }
#define ASSERT(x) if(!(x)) { cout << "At 0x" << hex << f.tellg() << dec << " assert failed: " << STR_VALUE(x) << endl; f.close(); exit(EXIT_SUCCESS); } // Allows to close all files properly when a new spectra is not complete (eof)

void read_binary(ifstream &f)
{
    unsigned long int value;  // will store values of size WORD_LENGTH

    f.ignore(TRIM);
    while(!f.eof()) {

        //**** RECORD 1: 32 words, spacecraft trajectory parameters, all floats
        f.ignore(2); // remove 0xC0 = 192 before record 1
        cout << "At 0x" << hex << f.tellg() << ":" << endl;

        for(int i=0; i<32 && !f.eof(); ++i) {
            f.read((char *)&value, WORD_LENGTH);
            //value = be64toh(value);
            //trvalue = read_apollo(be64toh(value));

            cout << "Trajectory parameter" << i+1 << ": " << value << endl;
        }

        //**** RECORD 2: 13 words, x-ray and gamma-ray common data, all ints except words 2 3 and 13 (floats)
        f.ignore(2); // remove 0x4E = 78 before record 2

        for(int i=0; i<13 && !f.eof(); ++i) {
            f.read((char *)&value, WORD_LENGTH);
            value = be64toh(value);

            if(i==0) {
                cout << "Record 2, GMT: " << dec << value << hex << endl;
            }

            //trvalue = read_apollo(be64toh(value));
        }

        //**** RECORD 3: 13 words, housekeeping data, merged ints/floats
        f.ignore(2); // remove 0x4E = 78 before record 3

        for(int i=0; i<13 && !f.eof(); ++i) {
            f.read((char *)&value, WORD_LENGTH);
            value = be64toh(value);

            if(i==0) {
                cout << "Record 3, GMT: " << dec << value << hex << endl;
           }

            //trvalue = read_apollo(be64toh(value));
        }

        //**** RECORD 4: 513 words, GMT followed by gamma-ray counts, all integers
        f.ignore(2); // remove 0xC06 = 3078 before record 4

        f.read((char *)&value, WORD_LENGTH);  // GMT

        for(int i=0; i<512 && !f.eof(); ++i) {
            f.read((char *)&value, WORD_LENGTH);
            value = be64toh(value);

            if(i==0) {
                cout << "Record 4, GMT: " << dec << value << hex << endl;
            }

            //if(value>0) cout << i << ": " << value << endl;
            //trvalue = read_apollo(be64toh(value));
        }
    }
}

int main(int argc, char **argv) {
    assert(sizeof(unsigned long int)>WORD_LENGTH); // Only 64 bit machines may handle enough space in only one value
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
