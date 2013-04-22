// chiplot text format - 4 line of headers followed by x y data lines
// Licence: Lesser GNU Public License 2.1 (LGPL)

// The format is trivial, as described here:
// http://www.esrf.eu/computing/scientific/FIT2D/FIT2D_REF/node115.html#SECTION0001851500000000000000

#ifndef XYLIB_CHIPLOT_H_
#define XYLIB_CHIPLOT_H_
#include "xylib.h"

namespace xylib {

    class ChiPlotDataSet : public DataSet
    {
        OBLIGATORY_DATASET_MEMBERS(ChiPlotDataSet)
    };

}
#endif // XYLIB_CHIPLOT_H_

