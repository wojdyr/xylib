// private helper functions (namespace xylib::util)
// Licence: Lesser GNU Public License 2.1 (LGPL)
// $Id$

#ifndef XYLIB_UTIL_H_
#define XYLIB_UTIL_H_

#include <string>
#include <vector>
#include <stdexcept>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <cassert>
#include <cmath>

#include "xylib.h"

namespace xylib { namespace util {

void le_to_host(void *ptr, int size);

unsigned int read_uint32_le(std::istream &f);
unsigned int read_uint16_le(std::istream &f);
int read_int16_le(std::istream &f);
float read_flt_le(std::istream &f);
double read_dbl_le(std::istream &f);

char read_char(std::istream &f);
std::string read_string(std::istream &f, unsigned len);

template<typename T>
T from_le(const char* p)
{
    T val = *reinterpret_cast<const T*>(p);
    le_to_host(&val, sizeof(val));
    return val;
}

double from_pdp11(const char* p);

std::string str_trim(std::string const& str);
void str_split(std::string const& line, std::string const& sep,
               std::string &key, std::string &val);
bool str_startwith(const std::string &str_src, const std::string &ss);
std::string str_tolower(const std::string &str);
bool has_word(const std::string &sentence, const std::string &word);

std::string read_line(std::istream &is);
bool get_valid_line(std::istream &is, std::string &line, char comment_char);

void skip_whitespace(std::istream &f);
Column* read_start_step_end_line(std::istream& f);
Block* read_ssel_and_data(std::istream &f, int max_headers=0);

long my_strtol(const std::string &str);
double my_strtod(const std::string &str);

inline bool is_numeric(int c)
    { return isdigit(c) || c=='+' ||  c=='-' || c=='.'; }

/// Round real to integer.
inline int iround(double d) { return static_cast<int>(floor(d+0.5)); }

/// S() converts any type to string
template <typename T, int N>
std::string format1(const char* fmt, T t)
{
    char buffer[N];
    snprintf(buffer, N, fmt, t);
    buffer[N-1] = '\0';
    return std::string(buffer);
}

template <typename T>
inline std::string S(T k) {
    return static_cast<std::ostringstream&>(std::ostringstream() << k).str();
}
inline std::string S(bool b) { return b ? "true" : "false"; }
inline std::string S(char const *k) { return std::string(k); }
inline std::string S(char *k) { return std::string(k); }
inline std::string S(char const k) { return std::string(1, k); }
inline std::string S(std::string const &k) { return k; }
inline std::string S() { return std::string(); }

/// delete all objects handled by pointers and clear vector
template<typename T>
void purge_all_elements(std::vector<T*> &vec)
{
    for (typename std::vector<T*>::iterator i=vec.begin(); i!=vec.end(); ++i)
        delete *i;
    vec.clear();
}

/// Read numbers from the string.
/// returns the first not processed character (from s.c_str())
const char* read_numbers(std::string const& s,
                         std::vector<double>& row);
// split block if it has columns with different sizes
std::vector<Block*> split_on_column_length(Block* block);


// DataSet
inline void format_assert(DataSet const* ds, bool condition,
                          std::string const& comment = "")
{
    if (!condition)
        throw FormatError("Unexpected format for filetype: "
                          + std::string(ds->fi->name)
                          + (comment.empty() ? comment : "; " + comment));
}

class ColumnWithName : public Column
{
public:
    ColumnWithName(double step) : step_(step) {}
    virtual std::string const& get_name() const { return name_; }
    void set_name(std::string const& name) { name_ = name; }
    virtual double get_step() const { return step_; }
    virtual void set_step(double step) { step_ = step; }

private:
    double step_;
    std::string name_;
};

// column uses vector<double> to represent the data
class VecColumn : public ColumnWithName
{
public:
    VecColumn() : ColumnWithName(0.) {}

    // implementation of the base interface
    int get_point_count() const { return (int) data.size(); }
    double get_value (int n) const
    {
        if (n < 0 || n >= get_point_count())
            throw RunTimeError("index out of range in VecColumn");
        return data[n];
    }

    void add_val(double val) { data.push_back(val); }
    void add_values_from_str(std::string const& str, char sep=' ');
    double get_min() const;
    double get_max(int point_count=0) const;

protected:
    std::vector<double> data;
    mutable double min_val, max_val;

    void calculate_min_max() const;
};


// column of fixed-step data
class StepColumn : public ColumnWithName
{
public:
    double start;
    int count; // -1 means unlimited...

    // get_min() and get_max() work properly only if step_ >= 0
    StepColumn(double start_, double step_, int count_ = -1)
        : ColumnWithName(step_), start(start_), count(count_)
    {}

    int get_point_count() const { return count; }
    double get_value(int n) const
    {
        if (count != -1 && (n < 0 || n >= count))
            throw RunTimeError("point index out of range");
        return start + get_step() * n;
    }
    double get_min() const { return start; }
    double get_max(int point_count=0) const
    {
        assert (point_count != 0 || count != -1);
        int n = (count == -1 ? point_count : count);
        return get_value(n-1);
    }
};

} } // namespace xylib::util

#endif // XYLIB_UTIL_H_
