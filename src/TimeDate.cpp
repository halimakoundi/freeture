/*
								TimeDate.cpp

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*
*	This file is part of:	freeture
*
*	Copyright:		(C) 2014-2015 Yoan Audureau -- FRIPON-GEOPS-UPSUD
*
*	License:		GNU General Public License
*
*	FreeTure is free software: you can redistribute it and/or modify
*	it under the terms of the GNU General Public License as published by
*	the Free Software Foundation, either version 3 of the License, or
*	(at your option) any later version.
*	FreeTure is distributed in the hope that it will be useful,
*	but WITHOUT ANY WARRANTY; without even the implied warranty of
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*	GNU General Public License for more details.
*	You should have received a copy of the GNU General Public License
*	along with FreeTure. If not, see <http://www.gnu.org/licenses/>.
*
*	Last modified:		20/10/2014
*
*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

/**
* \file    TimeDate.cpp
* \author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
* \version 1.0
* \date    13/06/2014
* \brief   Manage time.
*/

#include "TimeDate.h"

TimeDate::TimeDate(){}

//http://rhubbarb.wordpress.com/2009/10/17/boost-datetime-locales-and-facets/
string TimeDate::localDateTime(::boost::posix_time::ptime pt, string format){

    string s;
    ostringstream datetime_ss;
    ::boost::posix_time::time_facet * p_time_output = new ::boost::posix_time::time_facet;
    locale special_locale (locale(""), p_time_output);
    // special_locale takes ownership of the p_time_output facet
    datetime_ss.imbue (special_locale);
    (*p_time_output).format(format.c_str()); // date time
    datetime_ss << pt;
    s = datetime_ss.str().c_str();
    // don't explicitly delete p_time_output
    return s;

}

//http://jean-paul.cornec.pagesperso-orange.fr/formule_jj.htm
double TimeDate::gregorianToJulian_1(vector<int> date){

    int y   = date.at(0);
    int m   = date.at(1);
    int d   = date.at(2);
    int h   = date.at(3);
    int min = date.at(4);
    int s   = date.at(5);

    if(m == 1 || m == 2){

        y = y - 1;
        m = m + 12;

    }

    double ent1 ;
    double fractpart1 = modf(y/100, &ent1);

    double ent2;
    double fractpart2 = modf(ent1/4, &ent2);
    double b = 2 - ent1 + ent2;

    double t = h/24 + min/1440 + s/86400;

    double ent3;
    double fractpart3 = modf(365.25*(y+4716),&ent3);

    double ent4;
    double fractpart4 = modf(30.6001*(m+1),&ent4);

    return ent3 + ent4 + d + t + b - 1524.5;

}

double TimeDate::gregorianToJulian_2(vector<int> date){

    int y   = date.at(0);
    int m   = date.at(1);
    int d   = date.at(2);

    if(m == 1 || m == 2){

        y = y - 1;
        m = m + 12;

    }

    double A, A1, B, C, D;
    double f0 = modf(y/100,&A);
    double f1 = modf(A/4,&A1);
    B = 2-A+A1;
    double f2 = modf(365.25*y,&C);
    double f3 = modf(30.6001*(m+1),&D);

    std::ostringstream strs;
    double val = B+C+D+d+1720994.5;
    strs << val;
    std::string str = strs.str();
    //cout <<std::setprecision(5)<< std::fixed<< val<<endl;

    return B+C+D+d+1720994.5;

}

double TimeDate::julianCentury(double julianDate){

    return (( julianDate - 2451545.0 ) / 36525 );

}

double TimeDate::hmsToHdecimal(int H, int M, int S){

    double res = H;

    double entPartM ;

    double fractPartM = modf(M/60,&entPartM);

    res = res + fractPartM;

    double entPartS;

    double fractPartS = modf(S/3600,&entPartS);

    res = res + fractPartS;

    return res;

}

double TimeDate::degreeToHdecimal(int val){

    return val / 15;

}

//conversion longitude degrées en H M S
vector <int> TimeDate::HdecimalToHMS(double val){

    vector<int> res;

    double entPart_h;
    double fractPart_ms_dec = modf(val,&entPart_h);

    double entPart_m;
    double fractPart_s_dec = modf(fractPart_ms_dec*60,&entPart_m);

    double entPart_s = fractPart_s_dec*60;

    res.push_back(entPart_h);
    res.push_back(entPart_m);
    res.push_back(entPart_s);

    return res;

}

double TimeDate::julianDateFromPreviousMidnightUT(int gregorianH, int gregorianMin, int gregorianS, double JDO){

    double H = gregorianH + ((double)gregorianMin)/60 + ((double)gregorianS)/3600;

    double JD = JDO + H/24;

    return JD;

}

//http://aa.usno.navy.mil/faq/docs/GAST.php
double TimeDate::localSideralTime_1(double JD0, int gregorianH, int gregorianMin, int gregorianS){

    double JD = JD0;

    double H  = gregorianH + gregorianMin/60 + gregorianS/3600;

    hmsToHdecimal(gregorianH, gregorianMin, gregorianS);

    double D    = JD - 2451545.0;

    double D0   = JD0 - 2451545.0;

    double T    = D/36525;

    double GMST = 6.697374558 + 0.06570982441908*D0 + 1.00273790935 * H + 0.000026 * T * T;

    // the Longitude of the ascending node of the Moon
    double omega = 125.04 - 0.052954 * D; // in degree

    // the Mean Longitude of the Sun
    double L = 280.04 - 0.98565 * D; // in degree

    //is the obliquity
    double epsilon = 23.4393 - 0.0000004*D; //in degree

    // the nutation in longitude, is given in hours approximately by
    double deltaPhi =  -0.000319 * sin(omega) - 0.000024 * sin( 2*L);

    //The equation of the equinoxes
    double eqeq = deltaPhi * cos(epsilon);

    double GAST = GMST + eqeq;

    return GAST;

}

//http://jean-paul.cornec.pagesperso-orange.fr/gmst0.htm
double TimeDate::localSideralTime_2(double julianCentury, int gregorianH, int gregorianMin, int gregorianS, int longitude){

    double T = julianCentury;

    //Temps sidéral de Greenwich à 0h TU
    double GTSM0_sec = 24110.54841 + (8640184.812866 * julianCentury) + (0.093104 * pow(julianCentury,2)) - (pow(julianCentury,3) * 0.0000062);
    //cout << "GTSM0_sec "<< GTSM0_sec<<endl;

    double entPart_h;
    double fractPart_ms_dec = modf(GTSM0_sec/3600,&entPart_h);
    //cout << "fractPart_ms_dec "<< fractPart_ms_dec<<endl;

    double entPart_m ;
    double fractPart_s = modf(fractPart_ms_dec*60,&entPart_m);
    //cout << "fractPart_s "<< fractPart_s<<endl;

    vector<int> GTSM0_hms;
    GTSM0_hms.push_back((int)entPart_h%24); // H
    GTSM0_hms.push_back((int)entPart_m); // M
	GTSM0_hms.push_back((int)std::floor(fractPart_s*60 + 0.5)); // S

    //cout << "GTSM0_hms-> "<<GTSM0_hms.at(0)<<"h "<<GTSM0_hms.at(1)<< "m "<<GTSM0_hms.at(2)<< "s"<<endl;

    //conversion longitude degrées en H décimal
   // double lon = 2.1794397;
    double lon_Hdec = longitude / 15;

    //cout << "LON_dec-> "<<lon_Hdec<<endl;

    //conversion longitude degrées en H M S
    double entPart_lon_h;
    double fractPart_lon_ms_dec = modf(lon_Hdec,&entPart_lon_h);

    double entPart_lon_m;
    double fractPart_lon_s_dec = modf(fractPart_lon_ms_dec*60,&entPart_lon_m);

    double entPart_lon_s = fractPart_lon_s_dec*60;

    //cout << "LON_hms-> "<<entPart_lon_h<<"h "<<entPart_lon_m<< "m "<<entPart_lon_s<< "s"<<endl;

    //cout << "LON_sec-> "<<entPart_lon_h*3600+entPart_lon_m*60+entPart_lon_s<<endl;


    //Conversion de l'heure en temps sidéral écoulé
    double HTS =(gregorianH*3600+gregorianMin*60+gregorianS)* 1.00273790935;
    //cout << "HTS_sec-> "<<HTS<<endl;


    double GTSMLocal_sec=GTSM0_sec+entPart_lon_h*3600+entPart_lon_m*60+entPart_lon_s+HTS;
    //cout << "GTSMLocal_sec-> "<<GTSMLocal_sec<<endl;


    double entPart_gtsm_h;
    double fractPart_gtsm_ms_dec = modf(GTSMLocal_sec/3600,&entPart_gtsm_h);
  //  cout << "fractPart_gtsm_ms_dec "<< fractPart_gtsm_ms_dec<<endl;

    double entPart_gtsm_m ;
    double fractPart_gtsm_s = modf(fractPart_gtsm_ms_dec*60,&entPart_gtsm_m);
  //  cout << "fractPart_gtsm_s "<< fractPart_gtsm_s<<endl;

    vector<double> GTSM0Local_hms;
    GTSM0Local_hms.push_back((double)((int)entPart_gtsm_h%24)); // H
    GTSM0Local_hms.push_back(entPart_gtsm_m); // M
    GTSM0Local_hms.push_back(fractPart_gtsm_s*60); // S

   // cout << "GTSMLocal_hms-> "<<GTSM0Local_hms.at(0)<<"h "<<GTSM0Local_hms.at(1)<< "m "<<fractPart_gtsm_s*60<< "s"<<endl;

    double sideralT = 0.0;
    sideralT = GTSM0Local_hms.at(0) * 15 + GTSM0Local_hms.at(1) * 0.250 + GTSM0Local_hms.at(2) *  0.00416667;

    //cout << "sideralT in degree-> "<<sideralT<<endl;

    return sideralT;

}

//http://jean-paul.cornec.pagesperso-orange.fr/gmst0.htm
double TimeDate::localSideralTime_2(double julianCentury, int gregorianH, int gregorianMin, float gregorianS, float longitude){

    double T = julianCentury;

    //Temps sidéral de Greenwich à 0h TU
    double GTSM0_sec = 24110.54841 + (8640184.812866 * julianCentury) + (0.093104 * pow(julianCentury,2)) - (pow(julianCentury,3) * 0.0000062);
    //cout << "GTSM0_sec "<< GTSM0_sec<<endl;

    double entPart_h;
    double fractPart_ms_dec = modf(GTSM0_sec/3600,&entPart_h);
    //cout << "fractPart_ms_dec "<< fractPart_ms_dec<<endl;

    double entPart_m ;
    double fractPart_s = modf(fractPart_ms_dec*60,&entPart_m);
    //cout << "fractPart_s "<< fractPart_s<<endl;

    vector<int> GTSM0_hms;
    GTSM0_hms.push_back((int)entPart_h%24); // H
    GTSM0_hms.push_back((int)entPart_m); // M
    GTSM0_hms.push_back((int)std::floor(fractPart_s*60+0.5)); // S

    //cout << "GTSM0_hms-> "<<GTSM0_hms.at(0)<<"h "<<GTSM0_hms.at(1)<< "m "<<GTSM0_hms.at(2)<< "s"<<endl;

    //conversion longitude degrées en H décimal
   // double lon = 2.1794397;
    double lon_Hdec = longitude / 15;

    //cout << "LON_dec-> "<<lon_Hdec<<endl;

    //conversion longitude degrées en H M S
    double entPart_lon_h;
    double fractPart_lon_ms_dec = modf(lon_Hdec,&entPart_lon_h);

    double entPart_lon_m;
    double fractPart_lon_s_dec = modf(fractPart_lon_ms_dec*60,&entPart_lon_m);

    double entPart_lon_s = fractPart_lon_s_dec*60;

    //cout << "LON_hms-> "<<entPart_lon_h<<"h "<<entPart_lon_m<< "m "<<entPart_lon_s<< "s"<<endl;

    //cout << "LON_sec-> "<<entPart_lon_h*3600+entPart_lon_m*60+entPart_lon_s<<endl;


    //Conversion de l'heure en temps sidéral écoulé
    double HTS =(gregorianH*3600+gregorianMin*60+gregorianS)* 1.00273790935;
    //cout << "HTS_sec-> "<<HTS<<endl;


    double GTSMLocal_sec=GTSM0_sec+entPart_lon_h*3600+entPart_lon_m*60+entPart_lon_s+HTS;
    //cout << "GTSMLocal_sec-> "<<GTSMLocal_sec<<endl;


    double entPart_gtsm_h;
    double fractPart_gtsm_ms_dec = modf(GTSMLocal_sec/3600,&entPart_gtsm_h);
  //  cout << "fractPart_gtsm_ms_dec "<< fractPart_gtsm_ms_dec<<endl;

    double entPart_gtsm_m ;
    double fractPart_gtsm_s = modf(fractPart_gtsm_ms_dec*60,&entPart_gtsm_m);
  //  cout << "fractPart_gtsm_s "<< fractPart_gtsm_s<<endl;

    vector<double> GTSM0Local_hms;
    GTSM0Local_hms.push_back((double)((int)entPart_gtsm_h%24)); // H
    GTSM0Local_hms.push_back(entPart_gtsm_m); // M
    GTSM0Local_hms.push_back(fractPart_gtsm_s*60); // S

   // cout << "GTSMLocal_hms-> "<<GTSM0Local_hms.at(0)<<"h "<<GTSM0Local_hms.at(1)<< "m "<<fractPart_gtsm_s*60<< "s"<<endl;

    double sideralT = 0.0;
    sideralT = GTSM0Local_hms.at(0) * 15 + GTSM0Local_hms.at(1) * 0.250 + GTSM0Local_hms.at(2) *  0.00416667;

    //cout << "sideralT in degree-> "<<sideralT<<endl;

    return sideralT;

}


vector<string> TimeDate::splitStringToStringVector(string str){

    vector<string> output;
    typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
    boost::char_separator<char> sep(":");
    tokenizer tokens(str, sep);
    for (tokenizer::iterator tok_iter = tokens.begin();tok_iter != tokens.end(); ++tok_iter){
        output.push_back(*tok_iter);
    }

    return output;

}

vector<int> TimeDate::splitStringToIntVector(string str){

    vector<string> output;
    vector<int> intOutput;
    typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
    boost::char_separator<char> sep(":");
    tokenizer tokens(str, sep);
    for (tokenizer::iterator tok_iter = tokens.begin();tok_iter != tokens.end(); ++tok_iter){
        output.push_back(*tok_iter);
    }


    for(int i = 0 ; i< output.size();i++){
       intOutput.push_back(atoi(output.at(i).c_str()));
    }

    return intOutput;

}

// Input date format : YYYY:MM:DD from YYYY-MM-DDTHH:MM:SS,fffffffff
// Output : vector<int> with YYYY, MM, DD, hh, mm, ss
vector<int> TimeDate::getIntVectorFromDateString(string date){

	vector<string> output1;
	vector<string> output2;
	vector<string> output3;
	vector<string> output4;
	vector<int> finalOuput;

	// Extract YYYY:MM:DD and HH:MM:SS,fffffffff from YYYY-MM-DDTHH:MM:SS,fffffffff
    typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
    boost::char_separator<char> sep1("T");
    tokenizer tokens1(date, sep1);
    for (tokenizer::iterator tok_iter = tokens1.begin();tok_iter != tokens1.end(); ++tok_iter){
        output1.push_back(*tok_iter);
    }

	// Extract YYYY, MM, DD from YYYY:MM:DD and convert them in integer.
	boost::char_separator<char> sep2("-");
	tokenizer tokens2(output1.front(), sep2);
    for (tokenizer::iterator tok_iter = tokens2.begin();tok_iter != tokens2.end(); ++tok_iter){
        output2.push_back(*tok_iter);
    }
    for(int i = 0 ; i< output2.size();i++){
       finalOuput.push_back(atoi(output2.at(i).c_str()));
    }

	// Extract hh:mm:ss from hh:mm:ss,fffffffff.
	boost::char_separator<char> sep3(",");
	tokenizer tokens3(output1.back(), sep3);
    for (tokenizer::iterator tok_iter = tokens3.begin();tok_iter != tokens3.end(); ++tok_iter){
        output3.push_back(*tok_iter);
    }

	// Extract hh, mm, ss from hh:mm:ss.
	boost::char_separator<char> sep4(":");
	tokenizer tokens4(output3.front(), sep4);
    for (tokenizer::iterator tok_iter = tokens4.begin();tok_iter != tokens4.end(); ++tok_iter){
        output4.push_back(*tok_iter);
    }
	for(int i = 0 ; i< output4.size();i++){
       finalOuput.push_back(atoi(output4.at(i).c_str()));
    }

	return finalOuput;

}

// Input date format : YYYY:MM:DD from YYYY-MM-DDTHH:MM:SS,fffffffff
string TimeDate::get_YYYYMMDD_fromDateString(string date){

	vector<string> output1;
	vector<string> output2;

	// Extract YYYY:MM:DD from YYYY-MM-DDTHH:MM:SS,fffffffff
    typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
    boost::char_separator<char> sep("T");
    tokenizer tokens1(date, sep);
    for (tokenizer::iterator tok_iter = tokens1.begin();tok_iter != tokens1.end(); ++tok_iter){
        output1.push_back(*tok_iter);
    }

	// Extract YYYY, MM, DD from YYYY:MM:DD
	boost::char_separator<char> sep1("-");
	tokenizer tokens2(output1.front(), sep1);
    for (tokenizer::iterator tok_iter = tokens2.begin();tok_iter != tokens2.end(); ++tok_iter){
        output2.push_back(*tok_iter);
    }

	// Build YYYYMMDD string.
	string yyyymmdd = "";
	for(int i = 0; i< output2.size(); i++){

		yyyymmdd += output2.at(i);

	}

    return yyyymmdd;

}

string TimeDate::get_YYYYMMDDThhmmss(string date){

	string finalDate = "";

	boost::char_separator<char> sep(".");
	boost::char_separator<char> sep1("T");
	boost::char_separator<char> sep2("-");
	boost::char_separator<char> sep3(":");

	// Extract YYYY-MM-DDTHH:MM:SS from YYYY-MM-DDTHH:MM:SS,fffffffff
	vector<string> output; // Two elements : YYYY-MM-DDTHH:MM:SS and fffffffff
    typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
    tokenizer tokens(date, sep);
    for (tokenizer::iterator tok_iter = tokens.begin();tok_iter != tokens.end(); ++tok_iter){
        output.push_back(*tok_iter);
    }

	// Extract YYYY-MM-DD and HH:MM:SS from YYYY-MM-DDTHH:MM:SS
	vector<string> output1; // Two elements : YYYY-MM-DD and HH:MM:SS
    tokenizer tokens1(output.front(), sep1);
    for (tokenizer::iterator tok_iter = tokens1.begin();tok_iter != tokens1.end(); ++tok_iter){
        output1.push_back(*tok_iter);
    }

	// Extract YYYY, MM, DD from YYYY-MM-DD
    tokenizer tokens2(output1.front(), sep2);
    for (tokenizer::iterator tok_iter = tokens2.begin();tok_iter != tokens2.end(); ++tok_iter){
		finalDate += (*tok_iter);
    }

	finalDate += "T";

	// Extract hh, mm, ss from HH:MM:SS
    tokenizer tokens3(output1.back(), sep3);
    for (tokenizer::iterator tok_iter = tokens3.begin();tok_iter != tokens3.end(); ++tok_iter){
        finalDate += (*tok_iter);
    }

	return finalDate;

}


