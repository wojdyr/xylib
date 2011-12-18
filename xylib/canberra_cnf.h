// Canberra CNF (aka CAM format), one of many variants
// Licence: Lesser GNU Public License 2.1 (LGPL)
// $Id: $

// based on:
//  - this converting script:
//    http://www.physicsforums.com/showpost.php?p=2279239&postcount=11
//  - analysis of a sample from Canberra DSA1000 MCA w/ Genie 2000 software.


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

