/*
 * apollo15.cc
 *
 *  Created on: Oct 26, 2015
 *      Author: ymollard
 */

#include "../mainframe_converter/mainframe_converter.h"

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

class Apollo15GammaRay: public MainframeConverter {
    private:
        string input_file;
        ifstream input_binary;
        ofstream output_csv;
        bool skip_spectra;
        unsigned int spectra_length;

    public:
        Apollo15GammaRay(string input_file, bool output_csv=false) {
            this->input_file = input_file;
            this->input_binary.open(input_file.c_str(), ios::in | ios::binary);
            if(output_csv) this->output_csv.open(input_file.append(".csv").c_str());
            this->skip_spectra = false;
            this->spectra_length = (32+13+13+512)*ENCODED_WORD_LENGTH_IBM_7044;
        }

        ~Apollo15GammaRay() {
            if(this->output_csv.is_open()) this->output_csv.close();
            if(this->input_binary.is_open()) this->input_binary.close();
        }

        bool eof() { // Not sure whether this EOF is a standard, so let's keep it here
            int eof_marker_length = 8;
            int eof_index = 0;
            u_int16_t int_val;
            bool is_eof = true;

            if(!this->input_binary.eof()) {
                while(eof_index<eof_marker_length) {
                    int_val = read_short_16b(this->input_binary);
                    if(int_val!=0x0909) {
                        is_eof = false;
                        break;
                    }
                    ++eof_index;
                }
                this->input_binary.seekg(-(eof_index+1)*2, ios_base::cur);
                return is_eof;
            }
            return true;
        }

        bool read_binary()
        {
            u_int64_t int_val;
            double float_val;

            this->input_binary.ignore(TRIM);
            while(!eof()) {
                this->skip_spectra = false;

                //**** RECORD 1: 32 words, spacecraft trajectory parameters, all floats
                this->input_binary.ignore(2); // remove 0xC0 = 192 before record 1
                cout << "At 0x" << hex << this->input_binary.tellg() << ":" << endl;

                for(int i=0; i<32; ++i) {
                    float_val = read_float_ibm_7044(this->input_binary);

                    if(i==0) {
                        int offset = check_consistency_and_align(this->input_binary, TYPE_FLOAT_IBM_7044, float_val,
                                                                 18050000., 18900000., -2, this->spectra_length);
                        if(offset==CONSISTENCY_FAILED) {
                            cerr << "Consistency checking GMT record 1 failed permanently, abandoning..." << endl;
                            return false;
                        }
                        else if(offset!=CONSISTENCY_PASSED) {
                            this->skip_spectra = true;
                            break;
                        }
                        cout << "Record 1, GMT: " << dec << fixed << float_val << endl;
                    }
                    //cout << "Trajectory parameter " << dec << i+1 << ": " << fixed << float_val << endl;
                    if(this->output_csv.is_open()) this->output_csv << fixed << float_val << ";";
                }

                //**** RECORD 2: 13 words, x-ray and gamma-ray common data, all ints except words 2 3 and 13 (floats)
                if(!this->skip_spectra)
                    this->input_binary.ignore(2); // remove 0x4E = 78 before record 2

                for(int i=0; i<13 && !this->skip_spectra; ++i) {
                    if(i==1 || i==2 || i==12) {  // These are floats
                        float_val = read_float_ibm_7044(this->input_binary);
                        if(this->output_csv.is_open()) this->output_csv << fixed << float_val << ";";
                    }
                    else {  // The rest is ints
                        int_val = read_int_ibm_7044(this->input_binary);
                        if(this->output_csv.is_open()) this->output_csv << fixed << int_val << ";";

//                        if(i==0) {
//                            int offset = check_consistency_and_align(this->input_binary, (int)int_val, 18050000, 18900000,
//                                                                     32*ENCODED_WORD_LENGTH_IBM_7044-4, this->spectra_length);
//                            if(offset==CONSISTENCY_FAILED) {
//                                cerr << "Consistency checking GMT record 2 failed permanently, abandoning..." << endl;
//                                return false;
//                            }
//                            else if(offset!=CONSISTENCY_PASSED) {
//                                this->skip_spectra = true;
//                                break;
//                            }
//                        }
                    }
                }

                //**** RECORD 3: 13 words, housekeeping data, merged ints/floats
                if(!this->skip_spectra)
                    this->input_binary.ignore(2); // remove 0x4E = 78 before record 3

                for(int i=0; i<13 && !this->skip_spectra; ++i) {
                    if(i==1 || i==2 || i==3 || i==7 || i==8 || i==9) {  // These are floats
                        float_val = read_float_ibm_7044(this->input_binary);
                        //cout << "Record 3 parameter " << dec << i+1 << ": " << int_val << endl;
                        if(this->output_csv.is_open()) this->output_csv << fixed << float_val << ";";
                    }
                    else {  // The rest is ints
                        int_val = read_int_ibm_7044(this->input_binary);
                        //cout << "Record 3 parameter " << dec << i+1 << ": " << int_val << endl;
                        if(this->output_csv.is_open()) this->output_csv << fixed << int_val << ";";

//                        if(i==0) {
//                            int offset = check_consistency_and_align(this->input_binary, (int)int_val, 18050000, 18900000,
//                                                                     45*ENCODED_WORD_LENGTH_IBM_7044-6, this->spectra_length);
//                            if(offset==CONSISTENCY_FAILED) {
//                                cerr << "Consistency checking GMT record 3 failed permanently, abandoning..." << endl;
//                                return false;
//                            }
//                            else if(offset!=CONSISTENCY_PASSED) {
//                                this->skip_spectra = true;
//                                break;
//                            }
//                            //cout << "Record 3, GMT: " << int_val << endl;
//                        }
                    }
                }

                //**** RECORD 4: 513 words, GMT followed by gamma-ray counts, all integers
                if(!this->skip_spectra) {
                    this->input_binary.ignore(2); // remove 0xC06 = 3078 before record 4

                    int_val = read_int_ibm_7044(this->input_binary);

//                    int offset = check_consistency_and_align(this->input_binary, (int)int_val, 18050000, 18900000,
//                                                             58*ENCODED_WORD_LENGTH_IBM_7044-8, this->spectra_length);
//                    if(offset==CONSISTENCY_FAILED) {
//                        cerr << "Consistency checking GMT record 4 failed permanently, abandoning..." << endl;
//                        return false;
//                    }
//                    else if(offset!=CONSISTENCY_PASSED) {
//                        this->skip_spectra = true;
//                        break;
//                    }
                    //cout << "Record 4, GMT: " << int_val << endl;

                    if(this->output_csv.is_open()) this->output_csv << fixed << int_val << ";";
                }

                for(int i=0; i<512 && !this->skip_spectra; ++i) {
                    int_val = read_int_ibm_7044(this->input_binary);
                    if(this->output_csv.is_open()) this->output_csv << fixed << int_val << ";";
                }

                if(this->output_csv.is_open()) {
                    if(this->skip_spectra)
                        this->output_csv << "(skipped corrupted record);";
                    this->output_csv << "\n";
                }
            }
            return true;
        }

        int run() {
            if(!this->input_binary.is_open())  { perror("unable to open file"); return EXIT_FAILURE; }
            if(this->read_binary()) {
                cout << "Conversion ended" << endl;
                return EXIT_SUCCESS;
            }
            else {
                cout << "Leaving conversion with failure" << endl;
                return EXIT_FAILURE;
            }
        }
};


int main(int argc, char **argv) {
    errHandler.name(argv[0]);

    openParameters("apollo15");
    gParameters->parseCommandLine(argc,argv);
    gParameters->checkMandatoryParameters();

    string input_file = stringParameter("inputfile"); /* input filename */
    return Apollo15GammaRay(input_file, true).run();
}
