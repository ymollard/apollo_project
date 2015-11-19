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
        ofstream output_csv;

    public:
        ApolloXRay(string input_file, bool output_csv=false) {
            this->input_file = input_file;
            this->input_binary.open(input_file.c_str(), ios::in | ios::binary);
            if(output_csv) this->output_csv.open(input_file.append(".csv").c_str());
        }

        ~ApolloXRay() {
            if(this->output_csv.is_open()) this->output_csv.close();
            if(this->input_binary.is_open()) this->input_binary.close();
        }

        void read_binary()
        {
            this->input_binary.ignore(TRIM);
            while(!this->input_binary.eof()) {
                u_int16_t block_size = read_short_16b(this->input_binary);
                cout << "At 0x" << hex << this->input_binary.tellg()-2 << " Block size " << dec << block_size << endl;
                this->input_binary.ignore(block_size);
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
    return ApolloXRay(input_file, true /* output CSV ? */).run();
}
