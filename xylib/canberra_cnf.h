// Canberra CNF (aka CAM format) with spectral data from Genie software
// Licence: Lesser GNU Public License 2.1 (LGPL)
// $Id$

// Based on code from Jim Fitzgerald that is used in FitzPeaks (Gamma Analysis
// and Calibration Software, http://www.jimfitz.demon.co.uk/fitzpeak.htm).
//
// I have rewritten large part of this code and may have introduced bugs.
// Let me know if you find one - Marcin W.
//
// The first attempt to add this format was based on script from:
//    http://www.physicsforums.com/showpost.php?p=2279239&postcount=11
// and on analysis of a sample from Canberra DSA1000 MCA w/ Genie 2000 software,
// but it was almost completely rewritten when JF sent me his code.


#ifndef XYLIB_CANBERRA_CNF_H_
#define XYLIB_CANBERRA_CNF_H_
#include "xylib.h"

namespace xylib {

    class CanberraCnfDataSet : public DataSet
    {
        OBLIGATORY_DATASET_MEMBERS(CanberraCnfDataSet)
    };

}
#endif // CANBERRA_CNF_H_

