// PANalytical XRDML
// Licence: Lesser GNU Public License 2.1 (LGPL)

#define BUILDING_XYLIB
#include "xrdml.h"

#include <cstdlib>
#include "util.h"

using namespace std;
using namespace xylib::util;

namespace xylib {

const FormatInfo XrdmlDataSet::fmt_info(
    "xrdml",
    "PANalytical XRDML",
    "xrdml",
    false,                     // whether binary
    true,                      // whether has multi-blocks
    &XrdmlDataSet::ctor,
    &XrdmlDataSet::check
);

bool XrdmlDataSet::check(istream &f, string*)
{
    return true;
}

void XrdmlDataSet::load_data(std::istream &f)
{
}

} // namespace xylib

