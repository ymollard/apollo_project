# Apollo project
This project aims at restoring scientific and technical data from Apollo missions generated and duplicated on IBM 360 and IBM 7044 machines. 

## Mainfraime converter
Abstract class containing tools for data manipulation.

## Gamma-ray spectrometer
Tape restoration introduced `00` bits every 6 bits in these files. Words are 36 bit long however after restoration their use 64 bit with these extras `00`. The method `MainframeConverter::to_8bits_ibm_7044(...)` removes extras null bits to read a proper value.

Some files were corrupted, i.e. an offset of several Bytes prevented some spectrums to be at the right location.
The method `MainframeConverter::check_consistency_and_align(...)` allows to detect corruption by considering a valid range for a value. If the checked value is outside the validity range, then corruption is detected and the method will find what is the next valid value of the same order of magnitude to realign the rest of the data properly. 

The format is described from 26-18 to 26-22 in the **Apollo Scientific Experiments Data Handbook**. 

## Mass spectrometer
The mass data are recorded in 3 records from 2 to 4. Some data print outs included in the documentation mention a record 1 that seems to have the same structure (but not exactly the same data) as record 4. The documentation also mentions contradictory record sizes in Bytes: 2817 (record 2), 2493 (record 3), 945 (record 4) against 2804, 2468, 916 for the other source. The actual files correspond to the latter and has been decoded succeffully using table 2 of document TODO, record 1 being record 4. It seems that the extra Bytes contain:
  * A 2-Byte duplicate of the record size
  * A 4-Byte identifier of the record type (2, 3 or 4)
  * Other Bytes that can't be identified but seem only noise (resp. 8, 20 and 24 Bytes for each record)

## X-ray spectrometer
Currently old code to be refactorized.

## Dependencies, compiling and running
The C++ code relys on the SAS library and uses qmake. SAS needs to be initialized before running the conversion tools.
```
source sas_init.sh
cd apollo_project/src
qmake -project main_mass.pro  # Generate the Makefile
make
./main_mass inputfile=../data/DD014191_F1.DAT
```
