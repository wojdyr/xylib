// CSV or DSV format (comma- or more generally delimiter-separated values)
// Licence: Lesser GNU Public License 2.1 (LGPL)
// $Id:$

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
// All non-numeric fields are read as NaNs.
// If the first line has more NaNs than 3rd and 4th lines
// it is assumed that this line is a header. Header is used for column titles.
// Default decimal point is '.'. If option 'decimal-comma' is set both '.'
// and ',' are regarded decimal point.
// Separator is determined automatically by reading the first 3 lines,
// trying 7 separators: "\t,;|:/ ". (If 'decimal-comma' is given, comma
// is not tried as separator.)
// The separator that gives most numbers in lines 3 and 4 is selected.

#ifndef XYLIB_CSV_H_
#define XYLIB_CSV_H_
#include "xylib.h"

namespace xylib {

    class CsvDataSet : public DataSet
    {
        OBLIGATORY_DATASET_MEMBERS(CsvDataSet)
        virtual bool is_valid_option(std::string const& t)
            { return t == "decimal-comma"; }
    };

}
#endif // XYLIB_CSV_H_

