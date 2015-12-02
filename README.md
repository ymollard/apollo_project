# Apollo project
This project aims at restoring scientific and technical data from Apollo missions generated and duplicated on IBM 360 and IBM 7044 machines. 

## Source code
### Mainfraime converter
Abstract class containing tools for data manipulation.

### Gamma-ray spectrometer
[Documentation](http://spdf.sci.gsfc.nasa.gov/pub/documents/old/documentation_from_nssdc/dataset_catalogs/DSC_0313.pdf)

Tape restoration introduced `00` bits every 6 bits in these files. Words are 36 bit long however after restoration their use 64 bit with these extras `00`. The method `MainframeConverter::to_8bits_ibm_7044(...)` removes extras null bits to read a proper value.

Some files were corrupted, i.e. an offset of several Bytes prevented some spectrums to be at the right location.
The method `MainframeConverter::check_consistency_and_align(...)` allows to detect corruption by considering a valid range for a value. If the checked value is outside the validity range, then corruption is detected and the method will find what is the next valid value of the same order of magnitude to realign the rest of the data properly. 

The format is described from 26-18 to 26-22 in the **Apollo Scientific Experiments Data Handbook**. 

### Mass spectrometer
[Documentation](http://spdf.sci.gsfc.nasa.gov/pub/documents/old/documentation_from_nssdc/dataset_catalogs/DSC_0283.pdf)

The mass data are recorded in 3 records from 2 to 4. Some data print outs included in the documentation mention a record 1 that seems to have the same structure (but not exactly the same data) as record 4. The documentation also mentions contradictory record sizes in Bytes: 2817 (record 2), 2493 (record 3), 945 (record 4) against 2804, 2468, 916 for the other source. The actual files correspond to the latter and has been decoded succeffully using table 2 of document TODO, record 1 being record 4. It seems that the extra Bytes contain:
  * A 2-Byte duplicate of the record size
  * A 4-Byte identifier of the record type (2, 3 or 4)
  * Other Bytes that can't be identified but seem only noise (resp. 8, 20 and 24 Bytes for each record)

### X-ray spectrometer
[Documentation](http://spdf.sci.gsfc.nasa.gov/pub/documents/old/documentation_from_nssdc/dataset_catalogs/DSC_0281.pdf)
[Scientific report](http://ntrs.nasa.gov/archive/nasa/casi.ntrs.nasa.gov/19720013147.pdf)

Currently old code to be refactorized.

## Reconvert binary files
The following procedure gives clues about how to run again the conversion of original data in CSV and/or FITS, according to the implementation and the requested output in argument.

### Dependencies
The C++ code relys on the SAS library and uses qmake. SAS is needed to write FITS files, convert from/to julian days, as well as open command line parameters. A subset of SAS is provided in folder `ctadev_64` for 64bit machines.

SAS relies on HEASOFT that must be compiled and installed (`cd BUILD_DIR && ./configure && make && make install`). In case the library can't be installed, paths to libs can be tweaked in file `libs.pri`.

Also make sure you have qt4 installed (package `qt4-dev`).

See the specific documentation of the dependencies:
 * [Heasoft (NASA)](http://heasarc.gsfc.nasa.gov/lheasoft/) (General-Use FTOOLS is enouth)
 * [SAS (ESA)](http://xmm.esac.esa.int/sas/) (Light version ctadev_64 included)

### Compiling and running
First of all, edit files `init.sh` and `libs.pri` to match your custom installation folder of HEASOFT. THe path to SAS (ctadev) must be fine since it's provided with the repository.

Then compile and run the conversion as is, using each time `inputfile=<path/to_file.DAT>` for the input file:
```
source init.sh  # Initializes SAS
cd apollo_project/src/mass_spectrometer
qmake # Generate the Makefile
make
./main_mass inputfile=../data/DD014191_F1.DAT
```
The default generates a CSV output alongside the original DAT file.

### Helper scripts
For numerous files, use the helper BASH scripts:
```
cd apollo_project/scripts
./retrieve_all_data.sh # Download the data files from NASA's FTP in the data/ folder with multiple calls to ./retrieve_generic.sh <url>
./generate_csv_gamma_ray.sh # Converts all gamma ray data files with multiple calls to ./main_gamma_ray inputfile=<file>
```

## Documentation
[Apollo Scientific Experiments Data Handbook](https://www.hq.nasa.gov/alsj/ApolloSciDataHndbk.pdf)
