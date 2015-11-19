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
        int apollo_num; // 15 or 16

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

        int get_record_type(ifstream &f) {
            short record_length = read_short_16b(f);
            short record_length_2 = read_short_16b(f);
            apollo_num = record_length==record_length_2? 15: 16;  /* Dirty trick to detect that before reading record 4, on apollo 16
                                                                    * data record_length==record_length_2+1 */

            u_int32_t record_type = read_int_ibm_360(f);

            u_int16_t value = read_short_16b(f);
            assert(value==0);

            switch(record_type) {
            case 0xafc:
                return 2;
            case 0x9b8:
                return 3;
            case 0x3ac:
                return 4;
            default:
                cout << "Unknown record " << hex << record_type << " at 0x" << hex << this->input_binary.tellg() << endl;

                return 0;
            }
        }

        void read_record_2() {
            cout << "At 0x" << hex << this->input_binary.tellg() << " record 2" <<  endl;

            for(int i=0; i<700 && !this->input_binary.eof(); ++i) {
                u_int32_t int_val = read_int_ibm_360(this->input_binary);

                //if(this->output_csv.is_open()) this->output_csv << fixed << int_val << ";";
            }
            this->input_binary.ignore(apollo_num==15? 8:9);
        }

        void read_record_3() {
            cout << "At 0x" << hex << this->input_binary.tellg() << " record 3" <<  endl;

            for(int i=0; i<616 && !this->input_binary.eof(); ++i) {
                u_int32_t int_val = read_int_ibm_360(this->input_binary);

                //if(this->output_csv.is_open()) this->output_csv << fixed << int_val << ";";
            }
            this->input_binary.ignore(apollo_num==15? 20:21);
        }

        void read_record_4() {
            cout << "At 0x" << hex << this->input_binary.tellg() << " record 4" <<  endl;

            for(int i=0; i<228 && !this->input_binary.eof(); ++i) {
                if(i==0 || i==1 || i>18 && i<213) {
                    u_int32_t int_val = read_int_ibm_360(this->input_binary);
                    if(this->output_csv.is_open()) this->output_csv << dec << fixed << int_val << ";";
                }
                else {
                    double float_val = read_float_ibm_360(this->input_binary);
                    if(this->output_csv.is_open()) this->output_csv << dec << fixed << float_val << ";";
                }
            }
            this->input_binary.ignore(apollo_num==15? 24:25);
        }

        void read_binary()
        {
            this->input_binary.ignore(TRIM);
            while(!this->input_binary.eof()) {

                int record_id = get_record_type(this->input_binary);
                switch(record_id) {
                case 2:
                    read_record_2();
                    break;
                case 3:
                    read_record_3();
                    break;
                case 4:
                    read_record_4();
                    if(this->output_csv.is_open()) this->output_csv << "\n";
                    break;
                default:
                    /* Unknown record type:
                     * Either EOF (almost) reached, or corruption
                     * TODO: Check these several hundreds of data near EOF
                     */
                    return;
                }
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

