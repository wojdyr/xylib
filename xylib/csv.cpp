// CSV or DSV format (comma- or more generally delimiter-separated values)
// Licence: Lesser GNU Public License 2.1 (LGPL)

#define BUILDING_XYLIB
#include "csv.h"
#include "util.h"
#include <algorithm>
#include <limits>

using namespace std;
using namespace xylib::util;

namespace xylib {

const FormatInfo CsvDataSet::fmt_info(
    "csv",
    "CSV or TSV",
    "csv tsv tab",
    false,                       // whether binary
    false,                       // whether has multi-blocks
    &CsvDataSet::ctor,
    &CsvDataSet::check,
    "decimal-comma"
);

static
bool is_space_or_end(const char* str)
{
    while (*str) {
        if (!isspace(*str))
            return false;
        ++str;
    }
    return true;
}

static
vector<string> split_csv_line(const string& line, char sep)
{
    vector<string> fields(1);
    bool in_quote = false;
    for (string::const_iterator i = line.begin(); i != line.end(); ++i) {
        if (*i == sep && !in_quote) {
            fields.push_back("");
        } else if (*i == '"') {
            in_quote = !in_quote;
        } else {
            if (*i == '\\' && i + 1 != line.end()) {
                // The text from CSV file is of secondary importance, so we
                // handle only a few escape sequences.
                if (*(i+1) == '"' || *(i+1) == sep || *(i+1) == '\\')
                    ++i;
                //if (*(i+1) == 'n') { fields.back() += '\n'; ++i; continue; }
            }
            fields.back() += *i;
        }
    }
    return fields;
}

static
int append_numbers_from_line(const string& line, char sep,
                             vector<vector<double> > *out)
{
    vector<string> t = split_csv_line(line, sep);
    out->resize(out->size() + 1);
    vector<double>& nums = out->back();
    if (out->size() > 0)
        nums.reserve(out->front().size());
    int number_count = 0;
    for (vector<string>::const_iterator i = t.begin(); i != t.end(); ++i) {
        const char* field = i->c_str();
        // If the field contains anything else than a number with optional
        // leading/trailing white-spaces then NaN is returned.
        char* endptr;
        double d = strtod(field, &endptr);
        if (endptr == field || !is_space_or_end(endptr))
            d = numeric_limits<double>::quiet_NaN();
        else
            number_count++;
        nums.push_back(d);
    }
    return number_count;
}

// count_csv_numbers() is used much less than append_numbers_from_line(),
// so we don't try to optimize it.
static
int count_csv_numbers(const string& line, char sep, int *number_count,
                      bool decimal_comma=false)
{
    vector<vector<double> > out;
    if (decimal_comma) {
        string modified = line;
        replace(modified.begin(), modified.end(), ',', '.');
        *number_count = append_numbers_from_line(modified, sep, &out);
    } else {
        *number_count = append_numbers_from_line(line, sep, &out);
    }

    return out.size() == 1 ? out[0].size() : 0;
}

static
char read_4lines(istream &f, bool& decimal_comma,
                 vector<vector<double> > *out,
                 vector<string> *column_names)
{
    // We set a limit on the line length because if we get a large file
    // with no new lines we don't want to read it all.
    const int buflen = 1600;
    char buffer[1600]; // buflen
    string lines[4];
    buffer[buflen-1] = '\0';
    for (int line_no = 1, cnt = 0; cnt < 4; ++line_no) {
        f.getline(buffer, buflen);
        if (!f || buffer[buflen-1] != '\0')
            throw FormatError("reading line " + S(line_no) + " failed.");
        if (is_space_or_end(buffer))
            continue;
        lines[cnt] = buffer;
        if (decimal_comma)
            replace(lines[cnt].begin(), lines[cnt].end(), ',', '.');
        ++cnt;
    }

    // Determine separator.
    // The first line can be header. Second line should not be a header,
    // but just in case, let's check lines 3 and 4.
    double max_score = 0;
    int field_count = 0;
    char sep = 0;
    // the last duplicated ';' here is to auto-detect a popular variant:
    // ',' as decimal point and ';' as separator
    // (we try to detect it even if decimal_comma option was not specified)
    const char* separators = "\t,;|: ;";
    for (const char* isep = separators; *isep != '\0'; ++isep) {
        bool comma = false;
        // handle the special ,/; case described above
        if (*(isep+1) == '\0') {
            if (decimal_comma)
                continue;
            comma = true;
        }
        int num2, num3;
        int fields2 = count_csv_numbers(lines[2], *isep, &num2, comma);
        if (fields2 < 2)
            continue;
        int fields3 = count_csv_numbers(lines[3], *isep, &num3, comma);
        if (fields2 != fields3)
            continue;
        int nan_count = fields2 - num2 + fields3 - num3;
        double score = num2 + num3 - 1e-3 * nan_count;
        if (score > max_score) {
            max_score = score;
            field_count = fields2;
            sep = *isep;
            if (comma)
                // we can do this b/c we know it's the final cycle of this loop
                decimal_comma = true;
        }
    }

    // if the first row has labels (not numbers) read them as column names
    int num0;
    int fields0 = count_csv_numbers(lines[0], sep, &num0, decimal_comma);
    bool has_header = (fields0 == field_count && num0 == 0 &&
                       !str_startwith(lines[0], "# "));
    if (has_header && column_names) {
        *column_names = split_csv_line(lines[0], sep);
    }

    // add numbers from the first 4 lines to `out`
    if (out != NULL) {
        for (int i = (has_header ? 1 : 0); i != 4; ++i) {
            if (decimal_comma)
                replace(lines[i].begin(), lines[i].end(), ',', '.');
            int n = append_numbers_from_line(lines[i], sep, out);
            if (n == 0)
                out->pop_back();
        }
    }

    return sep;
}

bool CsvDataSet::check(istream &f, string* details)
{
    try {
        bool decimal_comma = false;
        char sep = read_4lines(f, decimal_comma, NULL, NULL);
        if (sep != 0 && details) {
            *details = "separator: " +
                       (sep == '\t' ? S("TAB") : "'" + S(sep) + "'");
            if (decimal_comma)
                *details += ", decimal comma";
        }
        return sep != 0;
    }
    catch (FormatError &) {
        return false;
    }
}


void CsvDataSet::load_data(istream &f, const char*)
{
    bool decimal_comma = has_option("decimal-comma");

    vector<vector<double> > data;
    vector<string> column_names;
    string line;
    line.reserve(100);

    char sep = read_4lines(f, decimal_comma, &data, &column_names);
    size_t n_col = data[0].size();
    while (getline(f, line)) {
        if (is_space_or_end(line.c_str()))
            continue;
        if (decimal_comma)
            replace(line.begin(), line.end(), ',', '.');
        int n = append_numbers_from_line(line, sep, &data);
        if (n == 0)
            data.pop_back();
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

