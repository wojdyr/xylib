// RIET7/LHPM/CSRIET DAT,  ILL_D1A5 and PSI_DMC formats
// Licence: Lesser GNU Public License 2.1 (LGPL)
// $Id$

#define BUILDING_XYLIB
#include "riet7.h"
#include "util.h"

using namespace std;
using namespace xylib::util;

namespace xylib {


const FormatInfo Riet7DataSet::fmt_info(
    "riet7",
    "RIET7/ILL_D1A5/PSI_DMC DAT",
    "dat",
    false,                      // whether binary
    false,                      // whether has multi-blocks
    &Riet7DataSet::ctor,
    &Riet7DataSet::check
);

static
int count_numbers(const char* p)
{
    int n = 0;
    while (*p != 0) {
        char *endptr;
        strtod(p, &endptr);
        if (p == endptr) // no more numbers
            break;
        ++n;
        p = endptr;
    }
    return n;
}

// .dat is popular extension for data, we want to avoid false positives.
// This format has line starting with three numbers: start step end
// somewhere in the first few lines, and it has intensities (in any format)
// in the following lines. We use here following rules:
// - try to read start-step-end from the first 6 lines:
//    one of the first 6 lines must start with three numbers, such that
//    n = (end - start) / step is an integer,
// - the start-step-end line and the next line must have different format
//   (if they have the same format, it's more likely xy format)
bool Riet7DataSet::check(istream &f)
{
    char line[256];
    for (int i = 0; i < 6; ++i) {
        f.getline(line, 255);
        int n = count_numbers(line);
        if (n < 3)
            continue;

        const char *startptr = line;
        char *endptr;
        double start = strtod(startptr, &endptr);
        startptr = endptr;
        double step = strtod(startptr, &endptr);
        startptr = endptr;
        double stop = strtod(endptr, &endptr);
        double dcount = (stop - start) / step + 1;
        int count = iround(dcount);
        if (count < 4 || fabs(count - dcount) > 1e-2)
            continue;

        f.getline(line, 255);
        int n2 = count_numbers(line);
        return n2 != n;
    }
    return false;
}

void Riet7DataSet::load_data(std::istream &f)
{
    Block *blk = read_ssel_and_data(f, 5);
    format_assert(this, blk != NULL);
    add_block(blk);
}

} // namespace xylib

