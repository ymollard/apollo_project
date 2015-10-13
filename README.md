# apollo_project
C++ hack to convert raw binaries from the NASA's Apollo 15/16 mission (generated IBM 360) to modern files

## APOLLO 15 transearth coast data C++ Converter

Difference between files:
* DR005893.F01: the original file from NSSDC
* DR005893.F01bis: the manually-corrected file
* DR005893.F02: the preprocessed file (preprocessed thanks to the "preprocess" program)

!! Important note for DR005893.F01bis:
As a perfect redundancy between records could not be observed in the whole file (maybe due to some errors occuring during copying
	from tape to disk or so) we manually corrected some Bytes so that the file could then be preprocessed easily. The following
	modifications have been manually brought to DR005893.F01:
* 4 extra Bytes 0xFFFFFFFF added at 0xFDF80
* 

### PREPROCESSING
The input file is DR005893.F01 (DATA DD014016). The 'preprocess' binary trims the file so that it begins to a proper record and
also removes all "block sizes", these 2-Bytes headers created by IBM-360 machines. It creates a DR005893.F02 file to input to the
'apollo15' program which will produce the actual FITS files.

### CONVERSION IN FITS
'apollo15' takes as arguments:
*   bool calibration:  If true, output calibration data instead of normal data (default: false)
*   bool sumcontrol: If true, put a negative flag to measurements failing at sum check (default: true)
*   int observation: Number of the observation to ouput (1 to 7 are the 7 Apollo 15 observations, 8 is the last ignition of the X-ray
	spectrometer when they discovered new sources before splashdown and 0 to output the whole data in a unique file) (default: 0)
*   string input: input file relative path and name (should not be DR005893.F01 but its preprocessed copy DR005893.F02) (default:
	./DR005893.F02)
It ouputs the FITS file(s) corresponding to the given observation.

### OTHER FILES
The modified version of Celestial.cc (from the SAS) can be used in case of the need of Euler231, Euler312, or Euler132. /* Warning: not fully
	implemented! Should not be included as is in the SAS */

