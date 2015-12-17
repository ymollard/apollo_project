/*
 * apollo15.cc
 *
 *  Created on: Jun 29, 2012
 *      Author: ymollard
 */
#include "../mainframe_converter/mainframe_converter.h"

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

#define TRIM 0

class ApolloXRay: public MainframeConverter {
    private:
        string input_file;
        ifstream input_binary;
        fstream temporary_binary;
        ofstream output_csv;

    public:
        ApolloXRay(string input_file, bool output_csv=false) {
            this->input_file = input_file;
            this->input_binary.open(input_file.c_str(), ios::in | ios::binary);
            this->temporary_binary.open((input_file + ".tmp").c_str(), ios::out | ios::binary);
            if(output_csv) this->output_csv.open((input_file + ".csv").c_str());
        }

        ~ApolloXRay() {
            if(this->temporary_binary.is_open()) this->temporary_binary.close();
            if(this->output_csv.is_open()) this->output_csv.close();
            if(this->input_binary.is_open()) this->input_binary.close();
        }

        void preprocess_binary()
        {
            //this->output_csv.open(input_file.append(".csv").c_str(), ios::binary););
            while(!this->input_binary.eof()) {
                u_int16_t block_size = read_short_16b(this->input_binary) - 2;
                u_int16_t block_size2 = read_short_16b(this->input_binary) - 2;

                cout << "At 0x" << hex << this->input_binary.tellg()-4 << " Block size " << dec << block_size << block_size2  << endl;
                assert(block_size==block_size2);

                this->input_binary.ignore(6);
                char block[block_size-6];
                this->input_binary.read(block, block_size-6);
                this->temporary_binary.write(block, block_size-6);
            }
        }

        int run() {
            if(!this->input_binary.is_open())  { perror("unable to open file"); return EXIT_FAILURE; }
            this->preprocess_binary();
            //this->read_binary();

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
    return ApolloXRay(input_file, true /* output CSV ? */).run();
}
