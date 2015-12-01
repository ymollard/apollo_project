#!/bin/bash

# GAMMA-RAY
./retrieve_generic.sh http://spdf.sci.gsfc.nasa.gov/pub/data/apollo/apollo15_csm/gamma-ray_spectrometer/gamma-ray_spectrometer_merge_data/

# X-RAY
# Apollo 15 Transearth coast
./retrieve_generic.sh http://spdf.sci.gsfc.nasa.gov/pub/data/apollo/apollo15_csm/x-ray_fluorescence/transearth_coast_x-ray_data/

# ORBITAL MASS
## Apollo 15
./retrieve_generic.sh http://spdf.sci.gsfc.nasa.gov/pub/data/apollo/apollo15_csm/orbital_mass_spectrometer/mass_spectrometer_data/
## Apollo 16
./retrieve_generic.sh http://spdf.sci.gsfc.nasa.gov/pub/data/apollo/apollo16_csm/orbital_mass_spectrometer/mass_spectrometer_data/
