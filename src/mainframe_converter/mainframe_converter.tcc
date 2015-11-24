#ifndef MAINFRAME_CONVERTER_TCC
#define MAINFRAME_CONVERTER_TCC

#include "mainframe_converter.h"

template <typename word>
int MainframeConverter::check_consistency_and_align(std::ifstream &f, int word_type, word value, word min_acceptable_value, word max_acceptable_value, off_t offset_to_new_record, off_t max_offset, bool debug) {
    /* Check the consistency of the file based on an acceptable range [min, max] for value.
     * In case value is outside this range, a maximum on max_offset will be skipped to align the file to the next record
     * max_offest should not be greater than the length of a record.
     * word_type is the type of the checked value, one of the TYPE_XXX_XXX constant
     * Returns CONSISTENCY_PASSED if the file is consistent so far, CONSISTENCY_EOF if EOF has been reached, CONSISTENCY_FAILED if alignment failed, or the applicated offest>0
     * If alignment was needed and succeeded according to the given range, the file is ready to read a new value of the same magnitude than "value".
    */
    if(value<min_acceptable_value || value>max_acceptable_value) {
        if(debug) std::cout << value << " not in acceptable range [" << min_acceptable_value << ", " << max_acceptable_value << "]" << std::endl;
        off_t offset=0;
        while(offset<max_offset && !f.eof()) {
            f.seekg(1, std::ios_base::cur);
            std::cout << "Testing value at 0x" << std::hex << f.tellg() << ": ";
            word new_value;
            int size;

            switch (word_type) {
            case TYPE_INT_IBM_360:
                new_value = read_int_ibm_360(f, debug);
                size = 4;
                break;
            case TYPE_FLOAT_IBM_360:
                new_value = read_float_ibm_360(f, debug);
                size = 4;
                break;
            case TYPE_INT_IBM_7044:
                new_value = read_int_ibm_7044(f, debug);
                size = 6;
                break;
            case TYPE_FLOAT_IBM_7044:
                new_value = read_float_ibm_7044(f, debug);
                size = 6;
                break;
            default:
                return CONSISTENCY_FAILED;
            }

            std::cout << new_value << std::endl;

            if(!(new_value<min_acceptable_value || new_value>max_acceptable_value)) {
                if(debug) std::cout << "Found new consistent value: " << std::fixed << new_value << " with an offset of " << std::dec << offset << std::endl;
                f.seekg(offset_to_new_record-size, std::ios_base::cur);  // Go to the new record from this inconsistent field
                return offset;
            }
            ++offset;
            f.seekg(-size, std::ios_base::cur);
        }
        if(f.eof())
            return CONSISTENCY_EOF; // No alignment needed when EOF has been reached
        else
            return CONSISTENCY_FAILED; // We tried to find a value of the same order of magnitude unsuccessfully given the maximum offset
    }
    else
        if(debug) std::cout << value << " in acceptable range [" << min_acceptable_value << ", " << max_acceptable_value << "]" << std::endl;
        return CONSISTENCY_PASSED; // No alignment needed when consistency checking passed
}

#endif // MAINFRAME_CONVERTER_TCC
