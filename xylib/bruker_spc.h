// Bruker ESP300-E raw binary spectrum
// Licence: Lesser GNU Public License 2.1 (LGPL)

// The implementation based on a function written by Christoph Burow
// https://github.com/tzerk/ESR/blob/master/R/read_Spectrum.R

#ifndef XYLIB_BRUKER_SPC_H_
#define XYLIB_BRUKER_SPC_H_

#include "xylib.h"

namespace xylib {

    class BrukerSpcDataSet : public DataSet
    {
        OBLIGATORY_DATASET_MEMBERS(BrukerSpcDataSet)
    };

} // namespace

#endif // XYLIB_BRUKER_SPC_H_

