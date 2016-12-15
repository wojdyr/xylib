// CSV or DSV format (comma- or more generally delimiter-separated values)
// Licence: Lesser GNU Public License 2.1 (LGPL)

// Here is a bunch of links related to CSV, TSV and in general DSV (comma-,
// tab- and delimiter-separated values):
// http://en.wikipedia.org/wiki/Comma-separated_values
// http://tools.ietf.org/html/rfc4180
// http://www.catb.org/~esr/writings/taoup/html/ch05s02.html
// http://www.iana.org/assignments/media-types/text/tab-separated-values
// 
// In this implementation continuation lines are not supported.
// Escape character is supported. Quoting is supported.
// Empty and blank lines are ignored.
//
// The delimiting character (separator) and decimal point are guessed from
// a few popular combinations by analysing (only) lines 3-4.
// If the option 'decimal-comma' is set - ',' is regarded a decimal point
// (in addition, not instead of '.').
//
// All non-numeric fields are read as NaNs.
// If the first line has the same number of fields, but more NaNs than 3rd
// and 4th lines it is assumed that this line is a header with column titles.
//
// Lines with all NaNs are ignored.

#ifndef XYLIB_CSV_H_
#define XYLIB_CSV_H_
#include "xylib.h"

namespace xylib {

    class CsvDataSet : public DataSet
    {
        OBLIGATORY_DATASET_MEMBERS(CsvDataSet)
    };

}
#endif // XYLIB_CSV_H_

