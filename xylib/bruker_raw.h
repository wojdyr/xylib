// Siemens/Bruker Diffrac-AT Raw Format version 1/2/3/4
// Licence: Lesser GNU Public License 2.1 (LGPL)

// Contains data from Siemens/Bruker X-ray diffractometers.
// Implementation based on:
// ver. 1 and 2: the file format specification from a diffractometer manual,
//               chapter "Appendix B: DIFFRAC-AT Raw Data File Format"
// ver. with magic string "RAW1.01", that probably is v. 4, because
//               corresponding ascii files start with ";RAW4.00",
//               was contributed by Andreas Breslau, who analysed binary files
//               and corresponding ascii files.
//               Later it was improved based on section
//               "A.1 DIFFRAC^plus V3 RAW File Structure" of the manual:
//               "DIFFRAC^plus FILE EXCHANGE and XCH" Release 2002.
// ver. with magic string "RAW4.00" by T.M. McQueen based on analysis
//               of binary files and corresponding ascii files.

#ifndef XYLIB_BRUKER_RAW_H_
#define XYLIB_BRUKER_RAW_H_

#include "xylib.h"

namespace xylib {

    class BrukerRawDataSet : public DataSet
    {
        OBLIGATORY_DATASET_MEMBERS(BrukerRawDataSet)

    protected:
        void load_version1(std::istream &f);
        void load_version2(std::istream &f);
        void load_version1_01(std::istream &f);
        void load_version4(std::istream &f);
    };

} // namespace

#endif // XYLIB_BRUKER_RAW_H_

