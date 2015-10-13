/*
 * apollo15.hpp
 *
 *  Created on: Jun 29, 2012
 *      Author: ymollard
 */

#ifndef ANALYSIS_HPP_
#define ANALYSIS_HPP_
#include <list>

#define A_SCALE1 scale==144? string("1.50 KeV < E < 2.24 KeV") : string("0.75 KeV < E < 1.03 KeV")
#define B_SCALE1 scale==144? string("2.25 KeV < E < 2.99 KeV") : string("1.04 KeV < E < 1.31 KeV")
#define C_SCALE1 scale==144? string("3.00 KeV < E < 3.74 KeV") : string("1.32 KeV < E < 1.60 KeV")
#define D_SCALE1 scale==144? string("3.75 KeV < E < 3.49 KeV") : string("1.61 KeV < E < 2.89 KeV")
#define E_SCALE1 scale==144? string("4.50 KeV < E < 4.24 KeV") : string("1.89 KeV < E < 2.18 KeV")
#define F_SCALE1 scale==144? string("5.25 KeV < E < 5.49 KeV") : string("2.18 KeV < E < 2.46 KeV")
#define G_SCALE1 scale==144? string("5.50 KeV < E < 6.00 KeV") : string("2.46 KeV < E < 2.75 KeV")
#define Z_SCALE1 scale==144? string("E > 6 KeV") : string("E > 2.75 KeV")

#define A_SCALE string("0.75 KeV < E < 1.03 KeV")
#define B_SCALE string("1.04 KeV < E < 1.31 KeV")
#define C_SCALE string("1.32 KeV < E < 1.60 KeV")
#define D_SCALE string("1.61 KeV < E < 2.89 KeV")
#define E_SCALE string("1.89 KeV < E < 2.18 KeV")
#define F_SCALE string("2.18 KeV < E < 2.46 KeV")
#define G_SCALE string("2.46 KeV < E < 2.75 KeV")
#define Z_SCALE string("E > 2.75 KeV")

#define BERYL string("Bare detector with Beryllium window ")
#define ALU string("Bare detector with Beryllium window and Aluminum filter ")
#define MAGN string("Bare detector with Beryllium window and Magnesium filter ")
#define SM string("Solar monitor ")

#define CTR_UNIT string("counts/8s")
#define VALID_UNIT string("n.d.")
#define VALID_COMMENT string("Validity of the associated value: non-zero if an error occurred (positive if reported as wrong by the spectrometer, negative if the sum of counts failed)")



using namespace std;

/* Sample for astronomic data */
struct sample {
	float get;
	unsigned int value;
	int scale; /* scale or PSD for housekeeping data */
	int validity;
};

float read_apollo(unsigned int value);
DataSet *write_fits(string filename, bool calib);
void read_binary(ifstream &f, bool calib, bool sumcontrol, unsigned int observation);

/****** Global variables (list of extracted values) ******/

list<struct sample> *b[8]; // Beryllium, for each channel A-G + Z
list<struct sample> *a[8]; // Aluminium, for each channel A-G + Z
list<struct sample> *m[8]; // Magnesium, for each channel A-G + Z
list<struct sample> *s[8]; // Solar monitor, for each channel A-G + Z

list<float> *gandn[39]; // GANDN

list<struct sample> *housekeeping[9]; // Housekeeping data, for each value measured
list<struct sample> *psd[3]; // Pulse shape discriminator for each detector

real32 *eph[39+2]; // 39 original GANDN + 2 computed alpha, delta
real32 *br32[8];
int32 *br32v[8];
real32 *mr32[8];
int32 *mr32v[8];
real32 *ar32[8];
int32 *ar32v[8];
real32 *sr32[8];
int32 *sr32v[8];
real32 *hkt;
int32 *hk[12]; // 9 for real housekeeping data + 3 for psd
int32 *hkv[12]; // 9 for real housekeeping data + 3 for psd


/*************************** Basic functions ******************************/

struct sample create_sample(float g, unsigned int v, int scale, int validity) {
	struct sample a = { g, v, scale, validity };
	return a;
}

string gandn_name(unsigned int i) {
	string t[39] = {"GMTs",
					"GETh",
					"GETs",
					"RevNo",
					"SelenLat",
					"SelenLong",
					"SelenRad",
					"Velocity",
					"ApoluneRad",
					"PeriluneRad",
					"ApoluneAlti",
					"PeriluneAlti",
					"MoonLookAngle_A",
					"MoonLookAngle_B",
					"Pitch",
					"Yaw",
					"Roll",
					"Pos_X",
					"Pos_Y",
					"Pos_Z",
					"Vel_X",
					"Vel_Y",
					"Vel_Z",
					"Acc_X",
					"Acc_Y",
					"Acc_Z",
					"Altitude",
					"AltitudeRate",
					"Acc_Xp",
					"Acc_Yp",
					"Acc_Zp",
					"Velo_Xp",
					"Velo_Yp",
					"Velo_Zp",
					"GeodAlt",
					"SunLook_A",
					"SunLook_B",
					"SunLookAngle_A",
					"SunLookAngle_B"};
	return t[i];
}

string gandn_units(unsigned int i) {
	string t[39] = {"sec.",
					"hr.",
					"sec.",
					"n.d.",
					"deg.",
					"deg.",
					"ft.",
					"fps",
					"ft.",
					"ft.",
					"n.mi.",
					"n.mi.",
					"deg.",
					"deg.",
					"deg.",
					"deg.",
					"deg.",
					"ft.",
					"ft.",
					"ft.",
					"fps",
					"fps",
					"fps",
					"ft/sec2",
					"ft/sec2",
					"ft/sec2",
					"ft.",
					"ft.",
					"ft/sec2",
					"ft/sec2",
					"ft/sec2",
					"fps",
					"fps",
					"fps",
					"n.mi.",
					"deg.",
					"deg.",
					"deg.",
					"deg."};
	return t[i];
}

string gandn_desc(unsigned int i) {
	string t[39] = {"Greenwich Mean Time from midnight preceding launch",
					"Ground Elapsed Time from launch",
					"Ground Elapsed Time from launch",
					"Current incremental revolution",
					"Selenographic latitude with 0deg being the equator of the moon",
					"Selenographic longitude from +180 to -180deg from east to west with 0deg referring to a line slightly to the west of Aristillus",
					"Selenographic radius",
					"Lunar inertial velocity",
					"Apolune radius",
					"Perilune radius",
					"Apolune altitude",
					"Perilune altitude",
					"Vehicle look angle to moon",
					"Vehicle look angle to moon",
					"Vehicle attitude with respect to local horizontal coordinate system",
					"Vehicle attitude with respect to local horizontal coordinate system",
					"Vehicle attitude with respect to local horizontal coordinate system. Note: In the vehicle nose forward (Pitch=0.0) orbital mode the Sim Bay center line has a Roll of +142.25deg. In the vehicle nose pointing backward (Pitch=-180) the Roll = -37.75deg",
					"Position component in lunar orbit, moon centered",
					"Position component in lunar orbit, moon centered",
					"Position component in lunar orbit, moon centered",
					"Velocity component in PACSS lunar orbit, moon centered",
					"Velocity component in PACSS lunar orbit, moon centered",
					"Velocity component in PACSS lunar orbit, moon centered",
					"Acceleration component in lunar orbit, moon centered",
					"Acceleration component in lunar orbit, moon centered",
					"Acceleration component in lunar orbit, moon centered",
					"Altitude above the lunar landing site",
					"Altitude rate in lunar orbit",
					"Sensed acceleration component in platform coordinates",
					"Sensed acceleration component in platform coordinates",
					"Sensed acceleration component in platform coordinates",
					"Sensed velocity component in platform coordinates",
					"Sensed velocity component in platform coordinates",
					"Sensed velocity component in platform coordinates",
					"Geodetic altitude",
					"Sun look angle in the sun vector system",
					"Sun look angle in the sun vector system",
					"Vehicle look angles to the sun",
					"Vehicle look angles to the sun"};
	return t[i];
}

/* return the name of the num th observation */
string cut_name(int num) {
	if(num <0 || num>8) num = 0;
	string names[9] = {"All", "Cen", "MGL", "Sco_X-1", "Cyg_X-1", "SGP", "NGP", "GPA", "NewSources"};
	return names[num];
}

/* return the GET when the num th observation starts */
float cut_start(int num) {
	if(num <0 || num>8) num = 0;
	float values[9] = { 0, 0, 837100, 870500, 887222, 918000, 968000, 889200, 994190 };
	return values[num];
}

/* return the GET when the num th observation stops */
float cut_stop(int num) {
	if(num <0 || num>8) num = 0;
	float values[9] = { 99999999, 837100, 870500, 887222, 918000, 968000, 889200, 994190, 99999999 };
	return values[num];
}

#endif /* ANALYSIS_HPP_ */
