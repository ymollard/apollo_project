/*
 * apollo15.cc
 *
 *  Created on: Jun 29, 2012
 *      Author: ymollard
 */

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
//#include "Celestial.h"

/* other includes */
#include "apollo15.hpp"
#include "jpleph.h"

//#include "Celestial.cc" // TODO dirty thing

using namespace Info;
using namespace std;

#define TRIM 			40 /* Bytes to remove to reach the first record in the file */
#define INSTRUMENT_ROLL	37.75*DEG_TO_RAD  /* instrument angle around the roll axis (x) */
#define JPLEPH_PATH 	"/home/ymollard/ctadev/lib/JPLEPH"
#define OUTPUT_PATH "./fits/"

#define STR_VALUE(arg) #arg
#define CHECK_END() if(feof(f)) { cout << "File ended" << endl; f.close(); dataSetServer->close(set); exit(EXIT_SUCCESS); }
#define ASSERT(x) if(!(x)) { cout << "At 0x" << hex << f.tellg() << dec << " assert failed: " << STR_VALUE(x) << endl; f.close(); exit(EXIT_SUCCESS); } // Allows to close all files properly when a new spectra is not complete (eof)

float read_apollo(unsigned int value) {
	/* Convert a 32-bits float value coming from the file into an IEEE-754 standard representation */
    unsigned int numchar, i, filter;
    float trvalue;
    numchar = (value & 0x0F000000) >> 24;


    /******** Read floor ********/
    filter = 0;
    for(i=0; i<numchar; i++) {
        filter |= (0xF<<(4*(5-i)));
    }
    //printf("floor filter: \t0x%X, numchar: %u\n", filter, numchar);
    trvalue = (value&filter)>>4*(6-numchar);

    /******** Read ceiling ********/
    filter = ~filter & 0x00FFFFFF;
    //printf("ceiling filter: \t0x%X\n", filter);
    double ceiling = 0;
    for(i=0; i<32; i++) {
        if(((value & filter) >> i) & 1) {
            ceiling += pow(2, i-24.0+4*numchar);
            //printf("adding 2^(%g) = %g\n", i-24.0+4*numchar, exp2(i-24.0+4*numchar));
            }
    }

    return ((value & 0xF0000000)==0xC0000000)? -trvalue-ceiling : trvalue+ceiling;
}


DataSet *write_fits(string filename, bool calib, int observation)
{
	/* Write the observation num "observation" in the output file "filename", "calib" is true if the output has to be calibration data
	 * It uses the global lists declared un the header file */

	unsigned int i, j, size;
	STime *reftime = new STime(1971, 7, 26, 13, 34, 0, 0.0); /*41159.065285;*/ /* 7/26/1971 13:34:00 Date/time of launch */
	STime *tlitime = new STime(1971, 7, 26, 16, 30, 3, 0.0); /*2441159.187535;*/ /* 7/26/1971 16:30:03 Trans lunar Injection */
	STime *teitime = new STime(1971, 8, 4, 21, 22, 45, 0.0); // 21:22:45 04 Aug 1971 /* Trans earth injection, whose line between Earth and Moon defining the spacecraft's coordinate system  */
	double tstart = gandn[0]->front();
	double tstop = gandn[0]->back();

    /********************************* Create FITS file *******************************/
    DataSet *set = dataSetServer->open(filename, Dal::Create);

    *set->addAttribute("TELESCOP") = "APOLLO15";
    *set->addAttribute("INSTRUME") = "X RAY SPECTROMETER";
/*    *set->addAttribute("RA");
    *set->addAttribute("DEC");
    *set->addAttribute("RA--NOM") = "";
    *set->addAttribute("DEC--NOM") = "";
    *set->addAttribute("EQUINOX");*/
    *set->addAttribute("OBJECT") = cut_name(observation).c_str();
    *set->addAttribute("RADECSYS") = "ICRS";
    *set->addAttribute("DATE-OBS") = "08/04/1971";
    *set->addAttribute("DATE-END") = "08/07/1971";
    *set->addAttribute("ORIGIN") = "ESA ESAC";
    *set->addAttribute("AUTHOR") = "HOMEMADE C++ CONVERTER";
    *set->addAttribute("TIMVERSN") = "OGIP/93-003";
    set->addComment("CONVERTED FROM NASA FILE DR005893.F01");
    set->addComment("APOLLO 15 TRANSEARTH COAST X-RAY DATA (1971)");
    set->addComment("Reference time for GET is launch 7/26/1971 13:34:00 UTC");
    set->addComment("Ref time for GMT is midnight preceding launch 7/26/1971 00:00:00 UTC");

    /**************************** Ephemeris table ********************************/
    Table *ephemeris = set->addTable("EPHEMERIS", gandn[0]->size());
    Column *col;
    for(i = 0;i < 39;++i){
        col = ephemeris->addColumn(gandn_name(i), Column::Real32, gandn_desc(i), gandn_units(i));
        eph[i] = col->data()->real32Data();
    }
    for(i = 0;i < 39;++i){
        size = gandn[i]->size();
        for(j = 0;j < size;++j){
            eph[i][j] = gandn[i]->front();
            gandn[i]->pop_front();
        }
    }

   /**************************** Determining where the instrument was pointing *******************/
//    Column *ra = ephemeris->addColumn("Right Ascension", Column::Real32, "Right Ascension", "deg");
//    eph[39] = ra->data()->real32Data();
//    Column *dec = ephemeris->addColumn("Declination", Column::Real32, "Declination", "deg");
//    eph[40] = dec->data()->real32Data();
//
//    double a_las, b_las, a_lam, b_lam;
//    double rrd[6];
//    int r;
//    double jdn;
//    double alpha_moon, delta_moon, alpha_sun, delta_sun;
//
//    void* e = jpl_init_ephemeris(JPLEPH_PATH, NULL, NULL);
//	if(!e) {
//		cerr << "Unable to compute moon coordinates. Error " << jpl_init_error_code() << " Check the JPLEPH path: " << JPLEPH_PATH << endl;
//		exit(EXIT_FAILURE);
//	}
//
//	for(j = 0;j < size;++j) {
//
//		//jdn = (*reftime + eph[2][j]).julianDay(STime::TT).full();  // current time
//		// jdn = tlitime->julianDay(STime::TT).full();  // Translunar injection /* !! Should NOT be used, the Apollo flight journal is wrong !! */
//		jdn = teitime->julianDay(STime::TT).full();  // Transearth injection
//
//		r = jpl_pleph(e, jdn, 10 /* moon */, 3 /* earth */, rrd, 0);
//		if(r) {
//			cerr << "Unable to compute moon coordinates." << endl;
//			exit(EXIT_FAILURE);
//		}
//
//		CartesianVector moon_vect(rrd[0], rrd[1], rrd[2]);
//		EquatorialDirection moon(moon_vect);
//		alpha_moon = moon.ra().angle();
//		delta_moon = moon.dec().angle();
//
//		r = jpl_pleph(e, jdn, 11 /* sun */, 3 /* earth */, rrd, 0);
//		if(r) {
//			cerr << "Unable to compute sun coordinates." << endl;
//			exit(EXIT_FAILURE);
//		}
//
//		CartesianVector sun_vect(rrd[0], rrd[1], rrd[2]);
//		EquatorialDirection sun(sun_vect);
//		alpha_sun = sun.ra().angle();
//		delta_sun = sun.dec().angle();
//
//		//cout << fixed << "At jd " << jdn << " moon was there: alpha = " << alpha_moon << "rad, delta = " << delta_moon << "rad" << endl;
//		//cout << fixed << "At jd " << jdn << " sun was there: alpha = " << alpha_sun << "rad, delta = " << delta_sun << "rad" << endl;
//
//		a_lam = eph[12][j]*DEG_TO_RAD;  /* alpha look angle to moon */
//		b_lam = eph[13][j]*DEG_TO_RAD;  /* beta look angle to moon */
//		a_las = eph[37][j]*DEG_TO_RAD;  /* alpha look angle to sun */
//		b_las = eph[38][j]*DEG_TO_RAD;  /* beta look angle to sun */
//
//		EulerAngles r1(alpha_moon, delta_moon - M_PI, 0., EulerAngles::Euler321);
//		EulerAngles r2(M_PI-b_lam+M_PI_2, a_lam, 0., EulerAngles::Euler123);
//		EulerAngles r3(-INSTRUMENT_ROLL, 0., 0., EulerAngles::Euler123);
//		AttitudeMatrix transformation(r3*r2*r1);
//		transformation = transformation.inverse();
//		CartesianVector instrument(0., 0., 1.);
//		CartesianVector instrument_icrf(transformation*instrument);
//		EquatorialDirection instrument_icrf_ad(instrument_icrf);
//		eph[39][j] = instrument_icrf_ad.ra().angle()*RAD_TO_DEG;
//		eph[40][j] = instrument_icrf_ad.dec().angle()*RAD_TO_DEG;
//	}
    /************************************************************************************************/


	/*  // Use it to ouput attitude during the specified GET hours (CSV format)
	  	cout << "GET (hr), ROLL (deg), PITCH (deg), YAW (deg)" << endl;
		for(j = 0;j < size;++j) {
			if(eph[1][j]<249. && eph[1][j]>245.) {
				pitch = eph[14][j];
				yaw = eph[15][j];
				roll = eph[16][j];
				cout <<	fixed << eph[1][j] << ", " << roll << ", " << pitch << ", " << yaw << endl;
			}
		}
	exit(0); */


    /******************** BERYLLIUM *******************/
    Table *beryllium = set->addTable("BERYLLIUM_RATE", b[0]->size());
    *beryllium->addAttribute("TELESCOP") = "APOLLO15";
    *beryllium->addAttribute("INSTRUME") = "X RAY SPECTROMETER";
    *beryllium->addAttribute("MJDREF") = reftime->modifiedJulianDay().full();
    *beryllium->addAttribute("TIMEZERO") = reftime->modifiedJulianDay().full();
    *beryllium->addAttribute("TSTART") = tstart;
    *beryllium->addAttribute("TSTOP") = tstop;
    *beryllium->addAttribute("FILTER") = "BERYLLIUM WIN NO FILTER";


    Column *btime = beryllium->addColumn("Time", Column::Real32, "GET", "s");
    real32 *bt = btime->data()->real32Data();

    Column *bscale = beryllium->addColumn("Scale", Column::Int32, "16 for normal mode 0.75KeV<E<3KeV, 144 for attenuate mode 1.5KeV<E<6KeV", VALID_UNIT);
    int32 *bsc = bscale->data()->int32Data();

    Column *beryllium1 = beryllium->addColumn("Be_Actr", Column::Real32, BERYL+A_SCALE, CTR_UNIT);
    br32[0] = beryllium1->data()->real32Data();

    Column *beryllium1v = beryllium->addColumn("Be_Actr_validity", Column::Int32, VALID_COMMENT, VALID_UNIT);
    br32v[0] = beryllium1v->data()->int32Data();

    Column *beryllium2 = beryllium->addColumn("Be_Bctr", Column::Real32, BERYL+B_SCALE, CTR_UNIT);
    br32[1] = beryllium2->data()->real32Data();

    Column *beryllium2v = beryllium->addColumn("Be_Bctr_validity", Column::Int32, VALID_COMMENT, VALID_UNIT);
    br32v[1] = beryllium2v->data()->int32Data();

    Column *beryllium3 = beryllium->addColumn("Be_Cctr", Column::Real32, BERYL+C_SCALE, CTR_UNIT);
    br32[2] = beryllium3->data()->real32Data();

    Column *beryllium3v = beryllium->addColumn("Be_Cctr_validity", Column::Int32, VALID_COMMENT, VALID_UNIT);
    br32v[2] = beryllium3v->data()->int32Data();

    Column *beryllium4 = beryllium->addColumn("Be_Dctr", Column::Real32, BERYL+D_SCALE, CTR_UNIT);
    br32[3] = beryllium4->data()->real32Data();

    Column *beryllium4v = beryllium->addColumn("Be_Dctr_validity", Column::Int32, VALID_COMMENT, VALID_UNIT);
    br32v[3] = beryllium4v->data()->int32Data();

    Column *beryllium5 = beryllium->addColumn("Be_Ectr", Column::Real32, BERYL+E_SCALE, CTR_UNIT);
    br32[4] = beryllium5->data()->real32Data();

    Column *beryllium5v = beryllium->addColumn("Be_Ectr_validity", Column::Int32, VALID_COMMENT, VALID_UNIT);
    br32v[4] = beryllium5v->data()->int32Data();

    Column *beryllium6 = beryllium->addColumn("Be_Fctr", Column::Real32, BERYL+F_SCALE, CTR_UNIT);
    br32[5] = beryllium6->data()->real32Data();

    Column *beryllium6v = beryllium->addColumn("Be_Fctr_validity", Column::Int32, VALID_COMMENT, VALID_UNIT);
    br32v[5] = beryllium6v->data()->int32Data();

    Column *beryllium7 = beryllium->addColumn("Be_Gctr", Column::Real32, BERYL+G_SCALE, CTR_UNIT);
    br32[6] = beryllium7->data()->real32Data();

    Column *beryllium7v = beryllium->addColumn("Be_Gctr_validity", Column::Int32, VALID_COMMENT, VALID_UNIT);
    br32v[6] = beryllium7v->data()->int32Data();

    Column *beryllium8 = beryllium->addColumn("Be_Zctr", Column::Real32, BERYL+Z_SCALE, CTR_UNIT);
    br32[7] = beryllium8->data()->real32Data();

    Column *beryllium8v = beryllium->addColumn("Be_Zctr_validity", Column::Int32, VALID_COMMENT, VALID_UNIT);
    br32v[7] = beryllium8v->data()->int32Data();

    for(i = 0;i < 8;++i){
        size = b[i]->size();
        for(j = 0;j < size;++j){
            br32[i][j] = b[i]->front().value;
            bt[j] = b[i]->front().get;
            bsc[j] = b[i]->front().scale;
            br32v[i][j] = b[i]->front().validity;
            b[i]->pop_front();
        }
    }

    /******************** MAGNESIUM *******************/
    Table *magnesium = set->addTable("MAGNESIUM_RATE", m[0]->size());
    *magnesium->addAttribute("TELESCOP") = "APOLLO15";
    *magnesium->addAttribute("INSTRUME") = "X RAY SPECTROMETER";
    *magnesium->addAttribute("MJDREF") = reftime->modifiedJulianDay().full();
    *magnesium->addAttribute("TIMEZERO") = reftime->modifiedJulianDay().full();
    *magnesium->addAttribute("TSTART") = tstart;
    *magnesium->addAttribute("TSTOP") = tstop;
    *magnesium->addAttribute("FILTER") = "BERYLLIUM WIN MAGNESIUM FILTERED";



    Column *mtime = magnesium->addColumn("Time", Column::Real32, "GET", "s");
    real32 *mt = mtime->data()->real32Data();

    Column *mscale = magnesium->addColumn("Scale", Column::Int32, "16 for normal mode 0.75KeV<E<3KeV, 144 for attenuate mode 1.5KeV<E<6KeV", "n.d.");
    int32 *msc = mscale->data()->int32Data();

    Column *magnesium1 = magnesium->addColumn("Mg_Actr", Column::Real32, MAGN+A_SCALE, CTR_UNIT);
    mr32[0] = magnesium1->data()->real32Data();

    Column *magnesium1v = magnesium->addColumn("Mg_Actr_validity", Column::Int32, VALID_COMMENT, VALID_UNIT);
    mr32v[0] = magnesium1v->data()->int32Data();

    Column *magnesium2 = magnesium->addColumn("Mg_Bctr", Column::Real32, MAGN+B_SCALE, CTR_UNIT);
    mr32[1] = magnesium2->data()->real32Data();

    Column *magnesium2v = magnesium->addColumn("Mg_Bctr_validity", Column::Int32, VALID_COMMENT, VALID_UNIT);
    mr32v[1] = magnesium2v->data()->int32Data();

    Column *magnesium3 = magnesium->addColumn("Mg_Cctr", Column::Real32, MAGN+C_SCALE, CTR_UNIT);
    mr32[2] = magnesium3->data()->real32Data();

    Column *magnesium3v = magnesium->addColumn("Mg_Cctr_validity", Column::Int32, VALID_COMMENT, VALID_UNIT);
    mr32v[2] = magnesium3v->data()->int32Data();

    Column *magnesium4 = magnesium->addColumn("Mg_Dctr", Column::Real32, MAGN+D_SCALE, CTR_UNIT);
    mr32[3] = magnesium4->data()->real32Data();

    Column *magnesium4v = magnesium->addColumn("Mg_Dctr_validity", Column::Int32, VALID_COMMENT, VALID_UNIT);
    mr32v[3] = magnesium4v->data()->int32Data();

    Column *magnesium5 = magnesium->addColumn("Mg_Ectr", Column::Real32, MAGN+E_SCALE, CTR_UNIT);
    mr32[4] = magnesium5->data()->real32Data();

    Column *magnesium5v = magnesium->addColumn("Mg_Ectr_validity", Column::Int32, VALID_COMMENT, VALID_UNIT);
    mr32v[4] = magnesium5v->data()->int32Data();

    Column *magnesium6 = magnesium->addColumn("Mg_Fctr", Column::Real32, MAGN+F_SCALE, CTR_UNIT);
    mr32[5] = magnesium6->data()->real32Data();

    Column *magnesium6v = magnesium->addColumn("Mg_Fctr_validity", Column::Int32, VALID_COMMENT, VALID_UNIT);
    mr32v[5] = magnesium6v->data()->int32Data();

    Column *magnesium7 = magnesium->addColumn("Mg_Gctr", Column::Real32, MAGN+G_SCALE, CTR_UNIT);
    mr32[6] = magnesium7->data()->real32Data();

    Column *magnesium7v = magnesium->addColumn("Mg_Gctr_validity", Column::Int32, VALID_COMMENT, VALID_UNIT);
    mr32v[6] = magnesium7v->data()->int32Data();

    Column *magnesium8 = magnesium->addColumn("Mg_Zctr", Column::Real32, MAGN+Z_SCALE, CTR_UNIT);
    mr32[7] = magnesium8->data()->real32Data();

    Column *magnesium8v = magnesium->addColumn("Mg_Zctr_validity", Column::Int32, VALID_COMMENT, VALID_UNIT);
    mr32v[7] = magnesium8v->data()->int32Data();


    for(i = 0;i < 8;++i){
        size = m[i]->size();
        for(j = 0;j < size;++j){
            mr32[i][j] = m[i]->front().value;
            mt[j] = m[i]->front().get;
            msc[j] = m[i]->front().scale;
            mr32v[i][j] = m[i]->front().validity;
            m[i]->pop_front();
        }
    }

    /******************** ALUMINIUM *******************/
    Table *aluminium = set->addTable("ALUMINIUM_RATE", a[0]->size());
    *aluminium->addAttribute("TELESCOP") = "APOLLO15";
    *aluminium->addAttribute("INSTRUME") = "X RAY SPECTROMETER";
    *aluminium->addAttribute("MJDREF") = reftime->modifiedJulianDay().full();
    *aluminium->addAttribute("TIMEZERO") = reftime->modifiedJulianDay().full();
    *aluminium->addAttribute("TSTART") = tstart;
    *aluminium->addAttribute("TSTOP") = tstop;
    *aluminium->addAttribute("FILTER") = "BERYLLIUM WIN ALUMINIUM FILTERED";


    Column *atime = aluminium->addColumn("Time", Column::Real32, "GET", "s");
    real32 *at = atime->data()->real32Data();

    Column *ascale = aluminium->addColumn("Scale", Column::Int32, "16 for normal mode 0.75KeV<E<3KeV, 144 for attenuate mode 1.5KeV<E<6KeV", "n.d.");
    int32 *asc = ascale->data()->int32Data();

    Column *aluminium1 = aluminium->addColumn("Al_Actr", Column::Real32, ALU+A_SCALE, CTR_UNIT);
    ar32[0] = aluminium1->data()->real32Data();

    Column *aluminium1v = aluminium->addColumn("Al_Actr_validity", Column::Int32, VALID_COMMENT, VALID_UNIT);
    ar32v[0] = aluminium1v->data()->int32Data();

    Column *aluminium2 = aluminium->addColumn("Al_Bctr", Column::Real32, ALU+B_SCALE, CTR_UNIT);
    ar32[1] = aluminium2->data()->real32Data();

    Column *aluminium2v = aluminium->addColumn("Al_Bctr_validity", Column::Int32, VALID_COMMENT, VALID_UNIT);
    ar32v[1] = aluminium2v->data()->int32Data();

    Column *aluminium3 = aluminium->addColumn("Al_Cctr", Column::Real32, ALU+C_SCALE, CTR_UNIT);
    ar32[2] = aluminium3->data()->real32Data();

    Column *aluminium3v = aluminium->addColumn("Al_Cctr_validity", Column::Int32, VALID_COMMENT, VALID_UNIT);
    ar32v[2] = aluminium3v->data()->int32Data();

    Column *aluminium4 = aluminium->addColumn("Al_Dctr", Column::Real32, ALU+D_SCALE, CTR_UNIT);
    ar32[3] = aluminium4->data()->real32Data();

    Column *aluminium4v = aluminium->addColumn("Al_Dctr_validity", Column::Int32, VALID_COMMENT, VALID_UNIT);
    ar32v[3] = aluminium4v->data()->int32Data();

    Column *aluminium5 = aluminium->addColumn("Al_Ectr", Column::Real32, ALU+E_SCALE, CTR_UNIT);
    ar32[4] = aluminium5->data()->real32Data();

    Column *aluminium5v = aluminium->addColumn("Al_Ectr_validity", Column::Int32, VALID_COMMENT, VALID_UNIT);
    ar32v[4] = aluminium5v->data()->int32Data();

    Column *aluminium6 = aluminium->addColumn("Al_Fctr", Column::Real32, ALU+F_SCALE, CTR_UNIT);
    ar32[5] = aluminium6->data()->real32Data();

    Column *aluminium6v = aluminium->addColumn("Al_Fctr_validity", Column::Int32, VALID_COMMENT, VALID_UNIT);
    ar32v[5] = aluminium6v->data()->int32Data();

    Column *aluminium7 = aluminium->addColumn("Al_Gctr", Column::Real32, ALU+G_SCALE, CTR_UNIT);
    ar32[6] = aluminium7->data()->real32Data();

    Column *aluminium7v = aluminium->addColumn("Al_Gctr_validity", Column::Int32, VALID_COMMENT, VALID_UNIT);
    ar32v[6] = aluminium7v->data()->int32Data();

    Column *aluminium8 = aluminium->addColumn("Al_Zctr", Column::Real32, ALU+Z_SCALE, CTR_UNIT);
    ar32[7] = aluminium8->data()->real32Data();

    Column *aluminium8v = aluminium->addColumn("Al_Zctr_validity", Column::Int32, VALID_COMMENT, VALID_UNIT);
    ar32v[7] = aluminium8v->data()->int32Data();


    for(i = 0;i < 8;++i) {
        size = a[i]->size();
        for(j = 0;j < size;++j) {
            ar32[i][j] = a[i]->front().value;
            at[j] = a[i]->front().get;
            asc[j] = a[i]->front().scale;
            ar32v[i][j] = a[i]->front().validity;
            a[i]->pop_front();
        }
    }

    if(!calib) {  // Solar monitor does not have any calibration data

		/******************** SOLAR MONITOR *******************/
		Table *solar = set->addTable("SOLAR_MONITOR_RATE", s[0]->size());
	    *solar->addAttribute("TELESCOP") = "APOLLO15";
	    *solar->addAttribute("INSTRUME") = "X RAY SPECTROMETER";
	    *solar->addAttribute("MJDREF") = reftime->modifiedJulianDay().full();
	    *solar->addAttribute("TIMEZERO") = reftime->modifiedJulianDay().full();
	    *solar->addAttribute("TSTART") = tstart;
	    *solar->addAttribute("TSTOP") = tstop;
	    *solar->addAttribute("FILTER") = "No";

		Column *stime = solar->addColumn("Time", Column::Real32, "GET", "s");
		real32 *st = stime->data()->real32Data();

		solar->addComment("Scale: Solar monitor always in normal mode 0.75KeV<E<3KeV");
		Column *solar1 = solar->addColumn("Solar_Actr", Column::Real32, SM+A_SCALE, CTR_UNIT);
		sr32[0] = solar1->data()->real32Data();

	    Column *solar1v = solar->addColumn("Solar_Actr_validity", Column::Int32, VALID_COMMENT, VALID_UNIT);
	    sr32v[0] = solar1v->data()->int32Data();

		Column *solar2 = solar->addColumn("Solar_Bctr", Column::Real32, SM+B_SCALE, CTR_UNIT);
		sr32[1] = solar2->data()->real32Data();

	    Column *solar2v = solar->addColumn("Solar_Bctr_validity", Column::Int32, VALID_COMMENT, VALID_UNIT);
	    sr32v[1] = solar2v->data()->int32Data();

		Column *solar3 = solar->addColumn("Solar_Cctr", Column::Real32, SM+C_SCALE, CTR_UNIT);
		sr32[2] = solar3->data()->real32Data();

	    Column *solar3v = solar->addColumn("Solar_Cctr_validity", Column::Int32, VALID_COMMENT, VALID_UNIT);
	    sr32v[2] = solar3v->data()->int32Data();

		Column *solar4 = solar->addColumn("Solar_Dctr", Column::Real32, SM+D_SCALE, CTR_UNIT);
		sr32[3] = solar4->data()->real32Data();

	    Column *solar4v = solar->addColumn("Solar_Dctr_validity", Column::Int32, VALID_COMMENT, VALID_UNIT);
	    sr32v[3] = solar4v->data()->int32Data();

		Column *solar5 = solar->addColumn("Solar_Ectr", Column::Real32, SM+E_SCALE, CTR_UNIT);
		sr32[4] = solar5->data()->real32Data();

	    Column *solar5v = solar->addColumn("Solar_Ectr_validity", Column::Int32, VALID_COMMENT, VALID_UNIT);
	    sr32v[4] = solar5v->data()->int32Data();

		Column *solar6 = solar->addColumn("Solar_Fctr", Column::Real32, SM+F_SCALE, CTR_UNIT);
		sr32[5] = solar6->data()->real32Data();

	    Column *solar6v = solar->addColumn("Solar_Fctr_validity", Column::Int32, VALID_COMMENT, VALID_UNIT);
	    sr32v[5] = solar6v->data()->int32Data();

		Column *solar7 = solar->addColumn("Solar_Gctr", Column::Real32, SM+G_SCALE, CTR_UNIT);
		sr32[6] = solar7->data()->real32Data();

	    Column *solar7v = solar->addColumn("Solar_Gctr_validity", Column::Int32, VALID_COMMENT, VALID_UNIT);
	    sr32v[6] = solar7v->data()->int32Data();

		Column *solar8 = solar->addColumn("Solar_Zctr", Column::Real32, SM+Z_SCALE, CTR_UNIT);
		sr32[7] = solar8->data()->real32Data();

	    Column *solar8v = solar->addColumn("Solar_Zctr_validity", Column::Int32, VALID_COMMENT, VALID_UNIT);
	    sr32v[7] = solar8v->data()->int32Data();

		for(i = 0;i < 8;++i){
			size = s[i]->size();
			for(j = 0;j < size;++j){
				sr32[i][j] = s[i]->front().value;
				st[j] = s[i]->front().get;
	            sr32v[i][j] = s[i]->front().validity;
				s[i]->pop_front();
			}
		}
    }


	Table *housek = set->addTable("HOUSEKEEPING_DATA", housekeeping[0]->size());

	Column *housekt = housek->addColumn("Time", Column::Real32, "GET", "s");
	hkt = housekt->data()->real32Data();

	Column *housek1 = housek->addColumn("Low_voltage_sum", Column::Int32, "Sum of all low voltage power supply outputs", "");
	hk[0] = housek1->data()->int32Data();

	Column *housek1v = housek->addColumn("Low_voltage_sum_validity", Column::Int32, VALID_COMMENT, "");
	hkv[0] = housek1v->data()->int32Data();

	Column *housek2 = housek->addColumn("Discriminator_ref_voltage", Column::Int32, "Discriminator reference voltage -12V", "");
	hk[1] = housek2->data()->int32Data();

	Column *housek2v = housek->addColumn("Discriminator_ref_voltage_validity", Column::Int32, VALID_COMMENT, "");
	hkv[1] = housek2v->data()->int32Data();

	Column *housek3 = housek->addColumn("analog_6_75V", Column::Int32, "+6.75V analog power supply", "");
	hk[2] = housek3->data()->int32Data();

	Column *housek3v = housek->addColumn("analog_6_75V_validity", Column::Int32, "validiy", "");
	hkv[2] = housek3v->data()->int32Data();

	Column *housek4 = housek->addColumn("digital_5V", Column::Int32, "+5V digital power supply", "");
	hk[3] = housek4->data()->int32Data();

	Column *housek4v = housek->addColumn("digital_5V_validity", Column::Int32, VALID_COMMENT, "");
	hkv[3] = housek4v->data()->int32Data();

	Column *housek5 = housek->addColumn("Processor_temp", Column::Int32, "X-ray processor assembly temperature", "");
	hk[4] = housek5->data()->int32Data();

	Column *housek5v = housek->addColumn("Processor_temp_validity", Column::Int32, VALID_COMMENT, "");
	hkv[4] = housek5v->data()->int32Data();

	Column *housek6 = housek->addColumn("Detector_temp", Column::Int32, "detector temperature", "");
	hk[5] = housek6->data()->int32Data();

	Column *housek6v = housek->addColumn("Detector_temp_validity", Column::Int32, VALID_COMMENT, "");
	hkv[5] = housek6v->data()->int32Data();

	Column *housek7 = housek->addColumn("Power_supply_temp", Column::Int32, "Low voltage power supply temperature", "");
	hk[6] = housek7->data()->int32Data();

	Column *housek7v = housek->addColumn("Power_supply_temp_validity", Column::Int32, VALID_COMMENT, "");
	hkv[6] = housek7v->data()->int32Data();

	Column *housek8 = housek->addColumn("Lunar_detector_temp", Column::Int32, "Lunar X-ray detector temperature", "");
	hk[7] = housek8->data()->int32Data();

	Column *housek8v = housek->addColumn("Lunar_detector_temp_validity", Column::Int32, VALID_COMMENT, "");
	hkv[7] = housek8v->data()->int32Data();

	Column *housek9 = housek->addColumn("Solar_detector_temp", Column::Int32, "Solar X-ray detector temperature", "");
	hk[8] = housek9->data()->int32Data();

	Column *housek9v = housek->addColumn("Solar_detector_temp_validity", Column::Int32, VALID_COMMENT, "");
	hkv[8] = housek9v->data()->int32Data();

	for(i = 0;i < 9;++i){
		size = housekeeping[i]->size();
		for(j = 0;j < size;++j){
			hkt[j] = housekeeping[i]->front().get;
			hk[i][j] = housekeeping[i]->front().value;
			hkv[i][j] = housekeeping[i]->front().validity;
			housekeeping[i]->pop_front();
		}
	}

	Column *housekpsd1 = housek->addColumn("PSD1", Column::Int32, "Pulse Shape Discriminator of detector 1 (Be)", "");
	hk[9] = housekpsd1->data()->int32Data();

	Column *housekpsd1v = housek->addColumn("PSD1_validity", Column::Int32, VALID_COMMENT, "");
	hkv[9] = housekpsd1v->data()->int32Data();

	Column *housekpsd2 = housek->addColumn("PSD2", Column::Int32, "Pulse Shape Discriminator of detector 2 (Mg)", "");
	hk[10] = housekpsd2->data()->int32Data();

	Column *housekpsd2v = housek->addColumn("PSD2_validity", Column::Int32, VALID_COMMENT, "");
	hkv[10] = housekpsd2v->data()->int32Data();

	Column *housekpsd3 = housek->addColumn("PSD3", Column::Int32, "Pulse Shape Discriminator of detector 3 (Al)", "");
	hk[11] = housekpsd3->data()->int32Data();

	Column *housekpsd3v = housek->addColumn("PSD3_validity", Column::Int32, VALID_COMMENT, "");
	hkv[11] = housekpsd3v->data()->int32Data();

	size = psd[0]->size();
	for(j = 0;j < size;j++){
		for(i=0;i<3;++i) {
			hk[i+9][j] = psd[i]->front().value;
			hkv[i+9][j] = psd[i]->front().validity;
			psd[i]->pop_front();
		}
	}

    return set;
}

void read_binary(ifstream &f, bool calib, bool sumcontrol, unsigned int observation)
{
	/* Read the binary stream "f" and fill the global lists with observation num "observation"
	 * "calib" is true if only calibration data has to be written in lists
	 *  "sumcontrol" is true if we want to set the validity flag to a negative value when sum of count rates does not match the specified sum
	 */

    unsigned int i, j, k, value, nspect, value2;
    int nextcalib = 0; /* Used to remove the next value after a calibration procedure, often wrong */
    float trvalue, sum; /* translated value */
    unsigned int xray[220]; /* The raw x-ray table in big-endian */
    list<struct sample>::reverse_iterator it; /* reverse iterator to change the validity flag in case of sum check failure */


    f.ignore(TRIM);
    while(!f.eof()) {
        f.ignore(4); // 4 useless Bytes? 0xA00000
        cout << "At 0x" << hex << f.tellg() << ":" << endl;
        for(i=0; i<39 && !f.eof(); ++i) {
        	f.read((char *)&value, 4);
        	trvalue = read_apollo(ntohl(value));
        	/*
        	if(i==2 && (trvalue < cut_start(observation) || trvalue > cut_stop(observation))) { // ignore headers that do not concern this observation (using the GETs column)
        		f.ignore(4*36);
        		gandn[0]->pop_back(); // remove GMT if this header does not concern this observation
        		gandn[1]->pop_back(); // remove GETh if this header does not concern this observation
        		break;
        	}
        	*/
            if(!i && (trvalue<800000 || trvalue> 1500000)) {
            	--i;
                f.seekg(-8, ios_base::cur); /* -4 Bytes to cancel the bad read and -4 Bytes to rewind */
                cout << "Warning: invalid header, rewinding 4 Bytes" << endl;
                continue;
            }
        	gandn[i]->push_back(trvalue);
        	if(!i) cout << "New header at gmt " << trvalue << endl;
        }

        f.read((char *)&value2, 4); // NSPECT OR 4 useless Bytes? 0x00080000

        f.read((char *)&value, 4); // NSPECT
        nspect = ntohl(value);

        if(nspect>1000u) {
            nspect = ntohl(value2);
            f.seekg(-4, ios_base::cur);
            cout << "Warning: invalid nspect, rewinding 4 Bytes" << endl;
        }

        cout << dec << "Expecting " << nspect << " spectras" << endl;
        ASSERT(nspect<1000u); // Sanity check

        for(i=0; i<nspect; ++i) {
        	try {
				f.ignore(4); // 4 useless Bytes? 0x3780
				cout << "At 0x" << hex << f.tellg() << ": ";
				f.read((char *)&value, 4); // GET

				trvalue = read_apollo(ntohl(value));
				//cout << "spectra " << i+1 << " at 0x" << hex << f.tellg() << dec << " GET " << trvalue << endl;
				f.read((char *)&xray, 220*4);
        	}
        	catch (...) {
				break;
			}
            if(!(trvalue > 800000 && trvalue < 1500000)) {
                cout << fixed << dec << "spectra " << i+1 << " at get " << trvalue << " invalid" << endl;
            }
            else if(trvalue > cut_start(observation) && trvalue < cut_stop(observation) &&
            		(calib && ((ntohl(xray[11])==96l || ntohl(xray[11])==224l || nextcalib)) ||
    				!calib && !nextcalib && ((ntohl(xray[11])==16l || ntohl(xray[11])==144l)))) {
					cout << dec << "spectra at get " << trvalue << " copied to FITS" << endl;
            	k = 0;
            	sum = 0;
            	for(j=13; j<29; j+=2) {
            		b[k++]->push_back(create_sample(trvalue, ntohl(xray[j]), ntohl(xray[11]), ntohl(xray[j-1])));
            		sum += ntohl(xray[j]);
            	}
            	k = 0;
            	for(j=29; j<45; j+=2) {
           			m[k++]->push_back(create_sample(trvalue, ntohl(xray[j]), ntohl(xray[11]), ntohl(xray[j-1])));
            		sum += ntohl(xray[j]);
            	}
            	k = 0;
            	for(j=45; j<61; j+=2) {
           			a[k++]->push_back(create_sample(trvalue, ntohl(xray[j]), ntohl(xray[11]), ntohl(xray[j-1])));
            		sum += ntohl(xray[j]);
            	}
				k = 0;
				for(j=61; j<77; j+=2) {
					s[k++]->push_back(create_sample(trvalue, ntohl(xray[j]), ntohl(xray[11]), ntohl(xray[j-1])));
            		sum += ntohl(xray[j]);
				}


				if(sumcontrol) {
					sum /= 16.0;
					int diff = abs((int)(ceil(sum)-ntohl(xray[7])));
					if(ceil(sum) != ntohl(xray[7])) {
						//cout << "Expected sum control of " << ceil(sum) << ", but read " <<  ntohl(xray[7]) << endl;
						for(k=0; k<8; ++k) {
							if(s[k]->back().validity == 0) s[k]->back().validity = -diff;
							if(a[k]->back().validity == 0) a[k]->back().validity = -diff;
							if(m[k]->back().validity == 0) m[k]->back().validity = -diff;
							if(b[k]->back().validity == 0) b[k]->back().validity = -diff;
						}

					}
				}


				/************************************ Store housekeeping data + psd **************************************/

					k = 0; int l;
					for(j=77;j<219;) {
						for(l=0; l<8; ++l) {
								housekeeping[k]->push_back(create_sample(trvalue+l, ntohl(xray[j]), 0, ntohl(xray[j-1])));
							j += 2;
						}
						++k;
					}

					for(k=0; k<8; ++k) {	/*	Duplicate the same data 8 times because
												the housekeeping data is set each second
												while the psd are set each 8 seconds */
						psd[0]->push_back(create_sample(0 /* GET coming from the housekeeping table */, ntohl(xray[1]), 0, ntohl(xray[0])));
						psd[1]->push_back(create_sample(0 /* GET coming from the housekeeping table */, ntohl(xray[3]), 0, ntohl(xray[2])));
						psd[2]->push_back(create_sample(0 /* GET coming from the housekeeping table */, ntohl(xray[5]), 0, ntohl(xray[4])));
					}

	            }
            else {
            	cout << dec << "spectra at get " << trvalue << " ignored due to given parameters (observation and/or calibration)" << endl;
            }
	            nextcalib = (ntohl(xray[11])==96l || ntohl(xray[11])==224l)? 1:0;

        }
    }
}

int main(int argc, char **argv) {
    assert(sizeof(unsigned int)==4);
    assert(sizeof(float)==4);

    errHandler.name(argv[0]);

    openParameters("apollo15");
    gParameters->parseCommandLine(argc,argv);
    gParameters->checkMandatoryParameters();

    int i;
    bool calib = booleanParameter("calibration"); /* If true, output calibration data instead of normal data */
    bool sumcontrol = booleanParameter("sumcontrol"); /* If true, put a negative flag to measurements failing at sum check */
    int observation = intParameter("observation");
    string filename = OUTPUT_PATH + cut_name(observation) + "_" + (calib? string("calibration_"):string("")) + string("apollo15.fits"); /* output file name */
    string input = stringParameter("inputfile"); /* input filename */

    ifstream f(input.c_str(), ios::in | ios::binary); // Input should be the file DR005893.F01 without Record sizes (use remove_blocks.cc to preprocess DR005893.F01)
    if(!f.is_open())  { perror("unable to open file"); exit(EXIT_FAILURE); }


    for(i = 0;i < 8;i++){
        b[i] = new list<struct sample>;
        m[i] = new list<struct sample>;
        a[i] = new list<struct sample>;
        s[i] = new list<struct sample>;
    }
    for(i = 0;i < 9;i++){
        housekeeping[i] = new list<struct sample>;
    }
    for(i = 0;i < 39;i++){
        gandn[i] = new list<float>;
    }
    for(i = 0;i < 3;i++){
        psd[i] = new list<struct sample>;
    }

    read_binary(f, calib, sumcontrol, observation);
    DataSet *set = write_fits(filename, calib, observation);

    for(i = 0;i < 8;i++){
        delete b[i];
        delete m[i];
        delete a[i];
        delete s[i];
    }

    for(i = 0;i < 39;i++){
        delete gandn[i];
    }
    for(i = 0;i < 9;i++){
        delete housekeeping[i];
    }
    for(i = 0;i < 3;i++){
    	delete psd[i];
    }

    dataSetServer->close(set);

    cout << "Conversion ended, output file: " << filename << endl;
    return EXIT_SUCCESS;
}
