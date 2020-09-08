// private helper functions (namespace xylib::util)
// Licence: Lesser GNU Public License 2.1 (LGPL)

#define BUILDING_XYLIB
#include "util.h"
#include "xylib.h"

#include <cassert>
#include <cctype>  // isspace
#include <cerrno>
#include <cmath>
#include <climits>
#include <cstdarg>
#include <cstdio>
#include <cstdlib> // strtol, strtod
#include <limits>
#include <boost/detail/endian.hpp>
#include <boost/cstdint.hpp>

#if !defined(BOOST_LITTLE_ENDIAN) && !defined(BOOST_BIG_ENDIAN)
#error "Unknown endianness"
#endif

using namespace std;
using namespace xylib::util;
using namespace boost;

namespace xylib { namespace util {

// -------   standard library functions with added error checking    --------

long my_strtol(const std::string &str)
{
    string ss = str_trim(str);
    const char *startptr = ss.c_str();
    char *endptr = NULL;
    long val = strtol(startptr, &endptr, 10);

    if (LONG_MAX == val || LONG_MIN == val) {
        throw FormatError("overflow when reading long");
    } else if (startptr == endptr) {
        throw FormatError("not an integer as expected");
    }

    return val;
}

double my_strtod(const std::string &str)
{
    const char *startptr = str.c_str();
    char *endptr = NULL;
    double val = strtod(startptr, &endptr);

    if (HUGE_VAL == val || -HUGE_VAL == val) {
        throw FormatError("overflow when reading double");
    } else if (startptr == endptr) {
        throw FormatError("not a double as expected");
    }

    return val;
}


// ----------   istream::read()- & endiannes-related utilities   ----------

namespace {
void my_read(istream &f, char *buf, int len)
{
    f.read(buf, len);
    if (f.gcount() < len) {
        throw FormatError("unexpected eof");
    }
}
} // anonymous namespace

// change the byte-order from "little endian" to host endian
// ptr: pointer to the data, size - size in bytes
#if defined(BOOST_BIG_ENDIAN)
void le_to_host(void *ptr, int size)
{
    char *p = (char*) ptr;
    for (int i = 0; i < size/2; ++i)
        swap(p[i], p[size-i-1]);
}
void be_to_host(void *, int) {}
#else
void le_to_host(void *, int) {}
void be_to_host(void *ptr, int size)
{
    char *p = (char*) ptr;
    for (int i = 0; i < size/2; ++i)
        swap(p[i], p[size-i-1]);
}
#endif

// read little-endian number from f and return equivalent in host endianess
template<typename T>
T read_le(istream &f)
{
    T val;
    my_read(f, reinterpret_cast<char*>(&val), sizeof(val));
    le_to_host(&val, sizeof(val));
    return val;
}

// read big-endian number from f and return equivalent in host endianess
template<typename T>
T read_be(istream &f)
{
    T val;
    my_read(f, reinterpret_cast<char*>(&val), sizeof(val));
    be_to_host(&val, sizeof(val));
    return val;
}

unsigned int read_uint32_le(istream &f) { return read_le<uint32_t>(f); }
int read_int32_le(istream &f) { return read_le<int32_t>(f); }
unsigned int read_uint16_le(istream &f) { return read_le<uint16_t>(f); }
int read_int16_le(istream &f) { return read_le<int16_t>(f); }
float read_flt_le(istream &f) { return read_le<float>(f); }
double read_dbl_le(istream &f) { return read_le<double>(f); }

int read_int32_be(istream &f) { return read_be<int32_t>(f); }

char read_char(istream &f)
{
    char val;
    my_read(f, &val, sizeof(val));
    return val;
}

// read a string from f
string read_string(istream &f, unsigned len)
{
    static char buf[256];
    assert(len < sizeof(buf));
    my_read(f, buf, len);
    buf[len] = '\0';
    return string(buf);
}

// function that converts single precision 32-bit floating point in DEC PDP-11
// format to double
// good description of the format is at:
// http://home.kpn.nl/jhm.bonten/computers/bitsandbytes/wordsizes/hidbit.htm
//
// TODO: consider this comment from S.M.:
//         PDP-11 F floating point numbers can be converted very easily (and
//         efficently) into standard IEEE 754 format: Swap the higher an lower
//         16-bit words, cast to float and divide by 4.
double from_pdp11(const unsigned char* p)
{
    int sign = (p[1] & 0x80) == 0 ? 1 : -1;
    int exb = ((p[1] & 0x7F) << 1) + ((p[0] & 0x80) >> 7);
    if (exb == 0) {
        if (sign == -1)
            // DEC calls it Undefined
            return numeric_limits<double>::quiet_NaN();
        else
            // either clean-zero or dirty-zero
            return 0.;
    }
    double h = p[2] / 256. / 256. / 256.
               + p[3] / 256. / 256.
               + (128 + (p[0] & 0x7F)) / 256.;
#if 0
    return ldexp(sign*h, exb-128);
#else
    return sign * h * pow(2., exb-128);
#endif
}


//    ----------       string utilities       -----------

// Return a copy of the string str with leading and trailing whitespace removed
string str_trim(string const& str)
{
    std::string ws = " \r\n\t";
    string::size_type first = str.find_first_not_of(ws);
    if (first == string::npos)
        return "";
    string::size_type last = str.find_last_not_of(ws);
    return str.substr(first, last - first + 1);
}


// skip whitespace, get key and val that are separated by `sep'
void str_split(string const& line, char sep, string &key, string &val)
{
    string::size_type p = line.find(sep);
    if (p == string::npos) {
        key = line;
        val = "";
    }
    else {
        key = str_trim(line.substr(0, p));
        val = str_trim(line.substr(p + 1));
    }
}

// change all letters in a string to lower case
std::string str_tolower(const std::string &str)
{
    string r(str);
    for (size_t i = 0; i != str.size(); ++i)
        r[i] = tolower(str[i]);
    return r;
}

// The `sentence' consists of space-separated words.
// Returns true if it contains `word'.
bool has_word(const string &sentence, const string& word)
{
    assert(!word.empty());
    size_t pos = 0;
    for (;;) {
        size_t found = sentence.find(word, pos);
        if (found == string::npos)
            return false;
        size_t end = found + word.size();
        if ((found == 0 || isspace(sentence[found-1])) &&
            (end == sentence.size() || isspace(sentence[end])))
            return true;
        pos = end;
    }
}


//      --------   line-oriented file reading functions   --------

// read a line and return it as a string
string read_line(istream& is)
{
    string line;
    if (!getline(is, line))
        throw FormatError("unexpected end of file");
    return line;
}

// get a trimmed line that is not empty and not a comment
bool get_valid_line(std::istream &is, std::string &line, char comment_char)
{
    size_t start = 0;
    while (1) {
        if (!getline(is, line))
            return false;
        start = 0;
        while (isspace(line[start]))
            ++start;
        if (line[start] && line[start] != comment_char)
            break;
    }
    size_t stop = start + 1;
    while (line[stop] && line[stop] != comment_char)
        ++stop;
    while (isspace(line[stop-1]))
        --stop;
    if (start != 0 || stop != line.size())
        line = line.substr(start, stop - start);
    return true;
}


void skip_whitespace(istream &f)
{
    while (isspace(f.peek()))
        f.ignore();
}

// read line (popular e.g. in powder data ascii file types) in free format:
// start step count
// example:
//   15.000   0.020 110.000
// returns NULL on error
Column* read_start_step_end_line(istream& f)
{
    char line[256];
    f.getline(line, 255);
    // the first line should contain start, step and stop
    char *endptr;
    const char *startptr = line;
    double start = strtod(startptr, &endptr);
    if (startptr == endptr)
        return NULL;

    startptr = endptr;
    double step = strtod(startptr, &endptr);
    if (startptr == endptr || step == 0.)
        return NULL;

    startptr = endptr;
    double stop = strtod(endptr, &endptr);
    if (startptr == endptr)
        return NULL;

    double dcount = (stop - start) / step + 1;
    int count = iround(dcount);
    if (count < 4 || fabs(count - dcount) > 1e-2)
        return NULL;

    return new StepColumn(start, step, count);
}

Block* read_ssel_and_data(istream &f, int max_headers)
{
    // we are looking for the first line with start-step-end numeric triple,
    // it should be one of the first (max_headers+1) lines
    Column *xcol = read_start_step_end_line(f);
    for (int i = 0; i < max_headers && xcol == NULL; ++i)
        xcol = read_start_step_end_line(f);

    if (!xcol)
        return NULL;

    Block* blk = new Block;
    blk->add_column(xcol);

    VecColumn *ycol = new VecColumn;
    string s;
    // in PSI_DMC there is a text following the data, so we read only as many
    // data lines as necessary
    while (getline(f, s) && ycol->get_point_count() < xcol->get_point_count())
        ycol->add_values_from_str(s);
    blk->add_column(ycol);

    // both xcol and ycol should have known and same number of points
    if (xcol->get_point_count() != ycol->get_point_count()) {
        delete blk;
        return NULL;
    }
    return blk;
}

// returns the first not processed character
const char* read_numbers(string const& s, vector<double>& row)
{
    row.clear();
    const char *p = s.c_str();
    while (*p != 0) {
        char *endptr = NULL;
        errno = 0; // to distinguish success/failure after call
        double val = strtod(p, &endptr);
        if (p == endptr) // no more numbers
            break;
        if (errno == ERANGE && (val == HUGE_VAL || val == -HUGE_VAL))
            throw FormatError("Numeric overflow in line:\n" + s);
        row.push_back(val);
        p = endptr;
        while (isspace(*p) || *p == ',' || *p == ';' || *p == ':')
            ++p;
    }
    return p;
}

// This function is used by pdCif code. In pdCif one block may contain
// columns of different lengths. In xylib all columns in one block must have
// the same length, so this function splits blocks when necessary.
// Columns are removed from `block', to prevent deleting it twice.
vector<Block*> split_on_column_length(Block *block)
{
    vector<Block*> result;
    while (block->get_column_count() > 0) {
        const int n = block->get_column(0).get_point_count();
        int r_idx = -1;
        if (n == -1)
            r_idx = (int) result.size() - 1;
        else
            for (size_t j = 0; j < result.size(); ++j) {
                if (result[j]->get_point_count() == n) {
                    r_idx = (int) j;
                    break;
                }
            }
        if (r_idx == -1) {
            r_idx = (int) result.size();
            Block* new_block = new Block;
            new_block->meta = block->meta;
            new_block->set_name(block->get_name() + (r_idx == 0 ? string()
                                                           : " " + S(r_idx)));
            result.push_back(new_block);
        }
        Column *col = block->del_column(0);
        result[r_idx]->add_column(col);
    }
    return result;
}


int count_numbers(const char* p)
{
    int n = 0;
    while (*p != '\0') {
        char *endptr;
        (void) strtod(p, &endptr);
        if (p == endptr) // no more numbers
            break;
        ++n;
        p = endptr;
    }
    return n;
}

void warn(const char *fmt, ...) {
    (void) fmt;
#ifndef DISABLE_STDERR_WARNINGS
    std::va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
#endif
}

// get all numbers in the first legal line
// sep is _optional_ separator that can be used in addition to white space
void VecColumn::add_values_from_str(string const& str, char sep)
{
    const char* p = str.c_str();
    while (isspace(*p) || *p == sep)
        ++p;
    while (*p != 0) {
        char *endptr = NULL;
        errno = 0; // To distinguish success/failure after call
        double val = strtod(p, &endptr);
        if (p == endptr)
            throw FormatError("Number not found in line:\n" + str);
        if (errno == ERANGE && (val == HUGE_VAL || val == -HUGE_VAL))
            throw FormatError("Numeric overflow in line:\n" + str);
        add_val(val);
        p = endptr;
        while (isspace(*p) || *p == sep)
            ++p;
    }
}

double VecColumn::get_min() const
{
    calculate_min_max();
    return min_val;
}

double VecColumn::get_max(int /*point_count*/) const
{
    calculate_min_max();
    return max_val;
}

void VecColumn::calculate_min_max() const
{
    // public api of VecColumn don't allow changing data, only appending
    if ((int) data.size() == last_minmax_length)
        return;

    if (data.empty()) {
        min_val = max_val = 0.;
        return;
    }
    min_val = max_val = data[0];
    for (vector<double>::const_iterator i = data.begin() + 1; i != data.end();
                                                                         ++i) {
        if (*i < min_val)
            min_val = *i;
        if (*i > max_val)
            max_val = *i;
    }
    last_minmax_length = data.size();
}

} } // namespace xylib::util

