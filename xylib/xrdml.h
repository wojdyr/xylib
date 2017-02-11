// PANalytical XRDML
// Licence: Lesser GNU Public License 2.1 (LGPL)

// A very well documented, XML-based format:
// http://www.panalytical.com/Xray-diffraction-software/Data-Collector/XRDML/Information-documents-XML-schema.htm

#ifndef XYLIB_XRDML_H_
#define XYLIB_XRDML_H_
#include "xylib.h"

namespace xylib {

    class XrdmlDataSet : public DataSet
    {
        OBLIGATORY_DATASET_MEMBERS(XrdmlDataSet)
    };

}
#endif // XYLIB_XRDML_H_

