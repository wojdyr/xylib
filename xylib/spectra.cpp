// Spectra data file
// Licence: Lesser GNU Public License 2.1 (LGPL)
// Author: Matthias Richter


#define BUILDING_XYLIB
#include "spectra.h"

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <iomanip>
#include "util.h"

using namespace std;
using namespace xylib::util;
using std::setprecision;
using std::fixed;



namespace {
    string doubleToString(double d){
        stringstream ss; 
        ss <<setprecision(15)<< d;
        return (ss.str());
    }
}

namespace xylib {


const FormatInfo SpectraDataSet::fmt_info(
    "spectra",
    "Spectra Omicron/Leybold data file",
    "1 2 3 4 5 6 7 8 9",       // file endings are numbered, number is increased for each new data aquisition in the same project
    false,                     // whether binary
    true,                      // whether has multi-blocks
    &SpectraDataSet::ctor,
    &SpectraDataSet::check
);

bool SpectraDataSet::check(istream &f, string*)
{
    string line = "";
    getline(f, line);  // first line should be experimentname
    if (line.size() < 1)
        return false;
    char line2[256];
    f.getline(line2, 255);  // second line should be parameters
    const char *pStart = line2;
    char * pEnd;

    strtod (pStart,&pEnd);
    if (*pEnd == 0)
        return false;
    pStart = pEnd;
    strtod (pStart,&pEnd);
    if (*pEnd == 0)
        return false;
    pStart = pEnd;
    strtod (pStart,&pEnd);
    if (*pEnd == 0)
        return false;
    pStart = pEnd;
    strtod (pStart,&pEnd);
    if (*pEnd == 0)
        return false;
    pStart = pEnd;
    strtod (pStart,&pEnd);
    if (*pEnd == 0)
        return false;
    pStart = pEnd;
    strtod (pStart,&pEnd);
    if (*pEnd == 0)
        return false;
    pStart = pEnd;
    strtod (pStart,&pEnd);
    if (*pEnd == 0)
        return false;
    getline(f, line);  // third line should be spectrumname
    if (line.size() < 1)
        return false;
    return true;
}



void SpectraDataSet::load_data(std::istream &f)
{

    string s = "";
    getline(f, s); // first line --> Experimentname

    bool read = true;
    while(read){
        Block* blk = NULL;
        double start    = 0.0;
        double ende     = 0.0;
        double step     = 0.0;
        double scans    = 0.0;
        double dwell    = 0.0;
        long points     = 0;
        double epass    = 0.0;
        double exenergy = 0.0;
        char line[256];
        const char *pStart = line;
        char * pEnd;
        f.getline(line, 255); // second line --> header
        if (f.eof()){
            read= false;
            break;
        }
        start       = strtod(pStart,&pEnd);
        pStart      = pEnd;
        ende        = strtod(pStart,&pEnd);
        pStart      = pEnd;
        step        = strtod(pStart,&pEnd);
        pStart      = pEnd;
        scans       = strtod(pStart,&pEnd);
        pStart      = pEnd;
        dwell       = strtod(pStart,&pEnd);
        pStart      = pEnd;
        points      = strtod(pStart,&pEnd);
        pStart      = pEnd;
        epass       = strtod(pStart,&pEnd);
        pStart      = pEnd;
        exenergy    = strtod(pStart,&pEnd);

        getline(f, s); // thrird line --> spectraname
        if (f.eof()){
            read= false;
            break;
        }
        blk = new Block;
        blk->set_name(s.c_str());
        blk->meta["Start"]          = doubleToString(start);
        blk->meta["Ende"]           = doubleToString(ende);
        blk->meta["step"]           = doubleToString(step);
        blk->meta["scans"]          = doubleToString(scans);
        blk->meta["dwell"]          = doubleToString(dwell);
        blk->meta["points"]         = doubleToString(points);
        blk->meta["EPass"]          = doubleToString(epass);
        blk->meta["Photon Energy"]  = doubleToString(exenergy);
        
        // StepColumn *xcol = new StepColumn(exenergy-start, -step); //positive binding energy
        StepColumn *xcol = new StepColumn(start-exenergy, step); //negative binding energy
        xcol->set_name("binding energy [eV]");
        blk->add_column(xcol);
        VecColumn *ycol = new VecColumn;
        ycol->set_name(s+" [cps]");
        for (long i = 0; i != points; ++i)
        {   
            getline(f, s);
            if (f.eof()){
                read= false;
                break;
            }
            ycol->add_val(my_strtod(s)/scans/dwell);
        }
        blk->add_column(ycol);
        add_block(blk);
    }
}

} // namespace xylib

