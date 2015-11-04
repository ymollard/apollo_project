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

        bool goto_next_record(ifstream &f, bool debug=true) {
            int num_padding_bytes = 0;
            short previous_padding=0;

            while(!f.eof()) {
                short padding = read_short_16b(f);
                if(padding==previous_padding && padding!=0) {
                    if(debug) cerr << dec << num_padding_bytes << " padding Bytes ignored until 0x" << hex << f.tellg() << endl;
                    f.seekg(-4, ios_base::cur);
                    return true;
                }
                previous_padding = padding;
                ++num_padding_bytes;
            }
            return false;
        }

        void read_binary()
        {
            u_int32_t int_val;
            double float_val;

            this->input_binary.ignore(TRIM);
            while(!this->input_binary.eof()) {

                //**** RECORD 1 (228 words) is in fact RECORD 4

                short record_2_length = read_short_16b(this->input_binary);
                assert(record_2_length==read_short_16b(this->input_binary));
                cout << "At 0x" << hex << this->input_binary.tellg() << " record 2 of size " << dec << record_2_length <<  endl;

                for(int i=0; i<700 && !this->input_binary.eof(); ++i) {
                    float_val = read_float_ibm_360(this->input_binary);

                    //if(this->output_csv.is_open()) this->output_csv << fixed << float_val << ";";
                }

                goto_next_record(this->input_binary);

                short record_3_length = read_short_16b(this->input_binary);
                assert(record_3_length==read_short_16b(this->input_binary));
                cout << "At 0x" << hex << this->input_binary.tellg() << " record 3 of size " << dec << record_3_length <<  endl;

                for(int i=0; i<616 && !this->input_binary.eof(); ++i) {
                    float_val = read_float_ibm_360(this->input_binary);

                    //if(this->output_csv.is_open()) this->output_csv << fixed << float_val << ";";
                }

                goto_next_record(this->input_binary);

                short record_4_length = read_short_16b(this->input_binary);
                assert(record_4_length==read_short_16b(this->input_binary));
                cout << "At 0x" << hex << this->input_binary.tellg() << " record 4 of size " << dec << record_4_length <<  endl;

                for(int i=0; i<228 && !this->input_binary.eof(); ++i) {
                    int_val = read_int_ibm_360(this->input_binary);

                    if(this->output_csv.is_open()) this->output_csv << hex << int_val << ";";
                }

                goto_next_record(this->input_binary);

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
    return Apollo15Mass(input_file, true).run();
}

