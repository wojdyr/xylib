// Spectra Omicron XPS data file
// Licence: Lesser GNU Public License 2.1 (LGPL)
// Author: Matthias Richter


#ifndef XYLIB_SPECTRA_H_
#define XYLIB_SPECTRA_H_
#include "xylib.h"

namespace xylib {

    class SpectraDataSet : public DataSet
    {
        OBLIGATORY_DATASET_MEMBERS(SpectraDataSet)
    private:
        Block* read_block(std::istream& f);
    };

}
#endif // XYLIB_SPECTRA_H_

