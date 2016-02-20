// ascii plain text
// Licence: Lesser GNU Public License 2.1 (LGPL)

#define BUILDING_XYLIB
#include "text.h"
#include <cstdlib>
#include "util.h"

using namespace std;
using namespace xylib::util;

namespace xylib {

const FormatInfo TextDataSet::fmt_info(
    "text",
    "Text",
    "", // "txt dat asc",
    false,                      // whether binary
    false,                      // whether has multi-blocks
    &TextDataSet::ctor,
    &TextDataSet::check,
    "strict first-line-header last-line-header decimal-comma"
);

bool TextDataSet::check(istream & /*f*/, string*)
{
    return true;
}

namespace {

// the title-line is either a name of block or contains names of columns
// we assume that it's the latter if the number of words is the same
// as number of columns
void use_title_line(string const& line, vector<VecColumn*> &cols, Block* blk)
{
    const char* delim = " \t";
    vector<string> words;
    std::string::size_type pos = 0;
    while (pos != std::string::npos) {
        std::string::size_type start_pos = line.find_first_not_of(delim, pos);
        pos = line.find_first_of(delim, start_pos);
        words.push_back(std::string(line, start_pos, pos-start_pos));
    }
    if (words.size() == cols.size()) {
        for (size_t i = 0; i < words.size(); ++i)
            cols[i]->set_name(words[i]);
    }
    else
        blk->set_name(line);
}

void replace_commas_with_dots(string &s)
{
    for (string::iterator p = s.begin(); p != s.end(); ++p)
        if (*p == ',')
            *p = '.';
}

} // anonymous namespace

void TextDataSet::load_data(std::istream &f)
{
    vector<VecColumn*> cols;
    vector<double> row; // temporary storage for values from one line
    string title_line;
    string s;

    bool strict = has_option("strict");
    bool first_line_header = has_option("first-line-header");
    // header is in last comment line - the line before the first data line
    bool last_line_header = has_option("last-line-header");
    bool decimal_comma = has_option("decimal-comma");

    if (first_line_header) {
        title_line = str_trim(read_line(f));
        if (!title_line.empty() && title_line[0] == '#')
            title_line = title_line.substr(1);
    }

    // read lines until the first data line is read and columns are created
    string last_line;
    while (getline(f, s)) {
        // Basic support for LAMMPS log file.
        // There is a chance that output from thermo command will be read
        // properly, but because the LAMMPS log file doesn't have
        // a well-defined syntax, it can not be guaranteed.
        // All data blocks (numeric lines after `run' command) should have
        // the same columns (do not use thermo_style/thermo_modify between
        // runs).
        if (!strict && str_startwith(s, "LAMMPS (")) {
            last_line_header = true;
            continue;
        }
        if (decimal_comma)
            replace_commas_with_dots(s);
        const char *p = read_numbers(s, row);
        // We skip lines with no data.
        // If there is only one number in first line, skip it if there
        // is a text after the number.
        if (row.size() > 1 ||
                (row.size() == 1 && (strict || *p == '\0' || *p == '#'))) {
            // columns initialization
            cols.reserve(row.size());
            for (size_t i = 0; i != row.size(); ++i) {
                cols.push_back(new VecColumn);
                cols[i]->add_val(row[i]);
            }
            break;
        }
        if (last_line_header) {
            string t = str_trim(s);
            if (!t.empty())
                last_line = (t[0] != '#' ? t : t.substr(1));
        }
    }

    // read all the next data lines (the first data line was read above)
    while (getline(f, s)) {
        if (decimal_comma)
            replace_commas_with_dots(s);
        read_numbers(s, row);

        // We silently skip lines with no data.
        if (row.empty())
            continue;

        if (row.size() < cols.size()) {
            // Some non-data lines may start with numbers. The example is
            // LAMMPS log file. The exceptions below are made to allow plotting
            // such a file. In strict mode, no exceptions are made.
            if (!strict) {
                // if it's the last line, we ignore the line
                if (f.eof())
                    break;

                // line with only one number is probably not a data line
                if (row.size() == 1)
                    continue;

                // if it's the single line with smaller length, we ignore it
                vector<double> row2;
                getline(f, s);
                if (decimal_comma)
                    replace_commas_with_dots(s);
                read_numbers(s, row2);
                if (row2.size() <= 1)
                    continue;
                if (row2.size() < cols.size()) {
                    // add the previous row
                    for (size_t i = 0; i != row.size(); ++i)
                        cols[i]->add_val(row[i]);
                    // number of columns will be shrinked to the size of the
                    // last row. If the previous row was shorter, shrink
                    // the last row.
                    if (row.size() < row2.size())
                        row2.resize(row.size());
                }
                // if we are here, row2 needs to be stored
                row = row2;
            }

            // this check is not redundant, row may have changed
            if (row.size() < cols.size()) {
                // decrease the number of columns to the new minimum
                for (size_t i = row.size(); i != cols.size(); ++i)
                    delete cols[i];
                cols.resize(row.size());
            }
        }

        else if (row.size() > cols.size()) {
            // Generally, we ignore extra columns. But if this is the second
            // data line, we ignore the first line instead.
            // Rationale: some data files have one or two numbers in the first
            // line, that can mean number of points or number of colums, and 
            // the real data starts from the next line.
            if (cols[0]->get_point_count() == 1) {
                purge_all_elements(cols);
                for (size_t i = 0; i != row.size(); ++i)
                    cols.push_back(new VecColumn);
            }
        }

        for (size_t i = 0; i != cols.size(); ++i)
            cols[i]->add_val(row[i]);
    }

    format_assert(this, cols.size() >= 1 && cols[0]->get_point_count() >= 2,
                  "data not found in file.");

    Block* blk = new Block;
    for (unsigned i = 0; i < cols.size(); ++i)
        blk->add_column(cols[i]);

    if (!title_line.empty())
        use_title_line(title_line, cols, blk);
    if (!last_line.empty())
        use_title_line(last_line, cols, blk);

    add_block(blk);
}

} // end of namespace xylib

