// chiplot text format - 4 line of headers followed by x y data lines
// Licence: Lesser GNU Public License 2.1 (LGPL)

#define BUILDING_XYLIB
#include "chiplot.h"

#include <cstdlib>
#include "util.h"

using namespace std;
using namespace xylib::util;

namespace xylib {

const FormatInfo ChiPlotDataSet::fmt_info(
    "chiplot",
    "ChiPLOT data",
    "chi",
    false,                      // whether binary
    false,                      // whether has multi-blocks
    &ChiPlotDataSet::ctor,
    &ChiPlotDataSet::check
);

bool ChiPlotDataSet::check(istream &f, string*)
{
    string line;
    for (int i = 0; i != 4; ++i)
        if (!getline(f, line))
            return false;
    // check 4. line
    char* endptr = NULL;
    int n = strtol(line.c_str(), &endptr, 10);
    if (endptr == line.c_str() || n <= 0)
        return false;
    // check 5. line
    getline(f, line);
    const char* p = line.c_str();
    (void) strtod(p, &endptr); // return value ignored intentionally
    if (endptr == p)
        return false;
    while (isspace(*p) || *p == ',')
        ++p;
    (void) strtod(p, &endptr); // return value ignored intentionally
    if (endptr == p)
        return false;
    return true;
}

/*
Description from 
http://www.esrf.eu/computing/scientific/FIT2D/FIT2D_REF/node115.html

The standard $\chi$PLOT data format is a very simple format allowing input of
one or more data-sets together with title and axis labels, and accompanying
text. Error estimates may be defined.  (...)

The minimum file format is as follows: (...)

The first line is the title of the graph, the second is the label to be written
for the X-axis, and the third line is the label for the Y-axis.  (...)

The fourth line specifies how many data points are present in each of the
data-sets, and how many data-sets to input.
This line should contain two integers separated by a one or more spaces or a
comma. If the second integer is missing it is assumed that only one curve is
required.

The next '*' lines of the file should contain the data to be plotted; where '*'
is the number of data points specified in line four. Each line should contain
the value for the X-coordinate, and the values of the Y-coordinates for the
different curves. (Note: All the curves share the same X-coordinates.)
(...) the values being separated by a blank space or a comma.  (...)

The following example shows a file which produces a single curve. Note on line
four of the file the number of curves is not specified so one is assumed.

Figure 1. SINGLE DATA-SET
X AXIS UNITS
Y AXIS UNITS
10
1 5e-2
2., 5.9e-2
3. 6.7e-2
4 6.8e-2
5 6.6e-2
6 5e-2
7 4.1e-2
8 2e-2
9 0.6e-2
10 -2.6e-2
*/

string trim_label(string const& str)
{
    string::size_type first = str.find_first_not_of(" \r\n\t#");
    if (first == string::npos)
        return "";
    string::size_type last = str.find_last_not_of(" \r\n\t");
    return str.substr(first, last - first + 1);
}

void ChiPlotDataSet::load_data(std::istream &f, const char*)
{
    string graph_title = trim_label(read_line(f));
    string x_label = trim_label(read_line(f));
    string y_label = trim_label(read_line(f));
    string line = read_line(f);
    string::size_type pos = line.find(',');
    if (pos != string::npos)
        line[pos] = ' ';
    int n_points, n_ycols;
    int r = sscanf(line.c_str(), "%d %d", &n_points, &n_ycols);
    if (r == 1)
        n_ycols = 1;
    else if (r != 2)
        throw FormatError("expected number(s) in line 4");
    if (n_points <= 0 || n_ycols <= 0)
        throw FormatError("expected positive number(s) in line 4");
    vector<VecColumn*> cols(n_ycols + 1);
    for (size_t i = 0; i != cols.size(); ++i)
        cols[i] = new VecColumn;
    try {
        for (int i = 0; i != n_points; ++i) {
            line = read_line(f);
            const char* p = line.c_str();
            for (int j = 0; j != n_ycols + 1; ++j) {
                char *endptr = NULL;
                while (isspace(*p) || *p == ',')
                    ++p;
                double val = strtod(p, &endptr);
                if (endptr == p)
                    throw FormatError("line " + S(5+i) + ", column " + S(j+1));
                cols[j]->add_val(val);
                p = endptr;
            }
        }
    }
    catch (std::exception&) {
        purge_all_elements(cols);
        throw;
    }

    Block *blk = new Block;
    blk->set_name(graph_title);
    cols[0]->set_name(x_label);
    cols[1]->set_name(y_label);
    for (size_t i = 0; i != cols.size(); ++i)
        blk->add_column(cols[i]);
    add_block(blk);
}

} // namespace xylib

