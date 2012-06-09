// CSV or DSV format (comma- or more generally delimiter-separated values)
// Licence: Lesser GNU Public License 2.1 (LGPL)
// $Id: $

#define BUILDING_XYLIB
#include "csv.h"
#include "util.h"
#include <limits>
#include <boost/tokenizer.hpp>

#include <boost/version.hpp>
#if BOOST_VERSION >= 103500
#include <boost/math/special_functions/fpclassify.hpp>
#else
namespace boost { namespace math { bool isnan(double f) { return f != f; } }}
#endif

using namespace std;
using namespace xylib::util;

namespace xylib {

typedef boost::tokenizer< boost::escaped_list_separator<char> > Tokenizer;

const FormatInfo CsvDataSet::fmt_info(
    "csv",
    "comma-separated values",
    "csv tsv tab",
    false,                       // whether binary
    false,                       // whether has multi-blocks
    &CsvDataSet::ctor,
    &CsvDataSet::check
);

// The field should contain only a number with optional leading/trailing
// white-spaces. For different input NaN is returned.
static
double read_field(const char* field)
{
    char* endptr;
    double d = strtod(field, &endptr);
    if (endptr != field) {
        while (isspace(*endptr))
            ++endptr;
        if (*endptr == '\0')
            return d;
    }
    return numeric_limits<double>::quiet_NaN();
}

static
int count_numbers(const string& line, char sep, int *number_count)
{
    int field_count = 0;
    *number_count = 0;
    Tokenizer t(line, boost::escaped_list_separator<char>('\\', sep, '"'));
    for (Tokenizer::iterator i = t.begin(); i != t.end(); ++i) {
        double d = read_field(i->c_str());
        ++field_count;
        if (!boost::math::isnan(d))
            ++(*number_count);
    }
    return field_count;
}

static
void read_numbers_from_line(const string& line, char sep, vector<double> *out)
{
    Tokenizer t(line, boost::escaped_list_separator<char>('\\', sep, '"'));
    for (Tokenizer::iterator i = t.begin(); i != t.end(); ++i)
        out->push_back(read_field(i->c_str()));
}

static
bool is_space(const char* str)
{
    while (*str) {
        if (!isspace(*str))
            return false;
        ++str;
    }
    return true;
}

static
char read_4lines(istream &f, bool decimal_comma,
                         vector<vector<double> > *out,
                         vector<string> *column_names)
{
    char buffer[1600]; // more than enough for one line
    string lines[4];
    buffer[1600-1]=0;
    for (int all_lines = 0, nonempty_lines=0;
            nonempty_lines < 4; ++all_lines) {
        // it can be a large file without new lines, limit line length
        f.getline(buffer, 1600);
        if (!f || buffer[1600-1] != 0)
            throw FormatError("reading line " + S(all_lines) + " failed.");
        if (!is_space(buffer)) {
            if (decimal_comma)
                for (char* p = buffer; *p != '\0'; ++p)
                    if (*p == ',')
                        *p = '.';
            lines[nonempty_lines] = buffer;
            ++nonempty_lines;
        }
    }

    // Determine separator.
    // The first line can be header. Second line should not be a header,
    // but just in case, let's check lines 3 and 4.
    int max_number_count = 0;
    int max_field_count = 0;
    char sep = 0;
    const char* separators = "\t,;|:/ ";
    for (const char* isep = separators; *isep != '\0'; ++isep) {
        int num2;
        int fields1 = count_numbers(lines[2], *isep, &num2);
        int num3;
        int fields2 = count_numbers(lines[3], *isep, &num3);
        if (fields1 == 0 || fields2 != fields1)
            continue;
        int num = min(num2, num3);
        if (num > max_number_count || (num == max_number_count &&
                                       fields1 > max_field_count)) {
            max_number_count = num;
            max_field_count = fields1;
            sep = *isep;
        }
    }

    // if the first row has labels (not numbers) read them as column names
    int num0;
    int fields0 = count_numbers(lines[0], sep, &num0);
    if (fields0 != max_field_count)
        throw FormatError("different field count (`" + S(sep) +
                          "'-separated) in lines 1 and 3.");
    bool has_header = (num0 < max_number_count);
    if (has_header && column_names) {
        Tokenizer t(lines[0],
                    boost::escaped_list_separator<char>('\\', sep, '"'));
        for (Tokenizer::iterator i = t.begin(); i != t.end(); ++i)
            column_names->push_back(*i);
    }

    // add numbers from the first 4 lines to `out`
    if (out != NULL) {
        if (!has_header) {
            out->resize(out->size() + 1);
            read_numbers_from_line(lines[0], sep, &out->back());
        }
        for (int i = 1; i != 4; ++i) {
            out->resize(out->size() + 1);
            read_numbers_from_line(lines[i], sep, &out->back());
        }
    }

    return sep;
}

bool CsvDataSet::check(istream &f)
{
    try {
        return read_4lines(f, false, NULL, NULL) != 0;
    }
    catch (FormatError &) {
        return false;
    }
}


void CsvDataSet::load_data(istream &f)
{
    bool decimal_comma = has_option("decimal_comma");

    vector<vector<double> > data;
    vector<string> column_names;
    string line;
    line.reserve(100);

    char sep = read_4lines(f, decimal_comma, &data, &column_names);
    size_t n_col = data[0].size();
    while (getline(f, line)) {
        if (is_space(line.c_str()))
            continue;
        if (decimal_comma)
            for (string::iterator p = line.begin(); p != line.end(); ++p)
                if (*p == ',')
                    *p = '.';
        data.resize(data.size() + 1);
        data.back().reserve(n_col);
        read_numbers_from_line(line, sep, &data.back());
    }

    Block* blk = new Block;
    for (size_t i = 0; i != n_col; ++i) {
        VecColumn *col = new VecColumn;
        if (column_names.size() > i)
            col->set_name(column_names[i]);
        col->reserve(data.size());
        for (size_t j = 0; j != data.size(); ++j)
            col->add_val(data[j].size() > i ? data[j][i]
                                      : numeric_limits<double>::quiet_NaN());
        blk->add_column(col);
    }
    add_block(blk);
}

} // namespace xylib

