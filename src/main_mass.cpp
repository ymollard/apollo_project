/*
 * apollo15.cc
 *
 *  Created on: Nov 3, 2015
 *      Author: ymollard
 */

#include "mainframe_converter.h"

/* system includes */
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

class Apollo15Mass: public MainframeConverter {
    private:
        string input_file;
        ifstream input_binary;
        ofstream output_csv;

    public:
        Apollo15Mass(string input_file, bool output_csv=false) {
            this->input_file = input_file;
            this->input_binary.open(input_file.c_str(), ios::in | ios::binary);
            if(output_csv) this->output_csv.open(input_file.append(".csv").c_str());
        }

        ~Apollo15Mass() {
            if(this->output_csv.is_open()) this->output_csv.close();
            if(this->input_binary.is_open()) this->input_binary.close();
        }

        void read_binary()
        {
            u_int32_t int_val;
            double float_val;

            this->input_binary.ignore(TRIM);
            while(!this->input_binary.eof()) {

                //**** RECORD 1: 228 words
                int record_1_length = read_int_ibm_360(this->input_binary);
                cout << "At 0x" << hex << this->input_binary.tellg() << " record of length " << dec << record_1_length << ":" << endl;

                for(int i=0; i<228 && !this->input_binary.eof(); ++i) {
                    float_val = read_float_ibm_360(this->input_binary, true);

                    if(i==0) {
                        cout << "Record 1, Apollo no: " << dec << fixed << float_val << endl;
                    }
                    //cout << "Trajectory parameter " << dec << i+1 << ": " << fixed << float_val << endl;
                    if(this->output_csv.is_open()) this->output_csv << fixed << float_val << ";";
                }

                //**** RECORD 2: 700 words
                this->input_binary.ignore(ENCODED_WORD_LENGTH_IBM_360); // Record 2 length

                for(int i=0; i<700 && !this->input_binary.eof(); ++i) {
                    float_val = read_float_ibm_360(this->input_binary);

                    if(this->output_csv.is_open()) this->output_csv << fixed << float_val << ";";
                }

                //**** RECORD 3: 616 words
                this->input_binary.ignore(ENCODED_WORD_LENGTH_IBM_360); // Record 3 length

                for(int i=0; i<616 && !this->input_binary.eof(); ++i) {
                    float_val = read_float_ibm_360(this->input_binary);

                    if(this->output_csv.is_open()) this->output_csv << fixed << float_val << ";";
                }

                if(this->output_csv.is_open()) this->output_csv << "\n";
            }
        }

        int run() {
            if(!this->input_binary.is_open())  { perror("unable to open file"); return EXIT_FAILURE; }
            this->read_binary();

            cout << "Conversion ended" << endl;
            return EXIT_SUCCESS;
        }
};


int main(int argc, char **argv) {
    errHandler.name(argv[0]);

    openParameters("apollo15");
    gParameters->parseCommandLine(argc,argv);
    gParameters->checkMandatoryParameters();

    string input_file = stringParameter("inputfile"); /* input filename */
    return Apollo15Mass(input_file, false).run();
}

