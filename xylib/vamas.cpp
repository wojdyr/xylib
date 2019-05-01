// ISO14976 VAMAS Surface Chemical Analysis Standard Data Transfer Format File
// Licence: Lesser GNU Public License 2.1 (LGPL)

#define BUILDING_XYLIB
#include "vamas.h"

#include <cstring>

#include "util.h"

using namespace std;
using namespace xylib::util;

namespace xylib {

const FormatInfo VamasDataSet::fmt_info(
    "vamas",
    "VAMAS ISO-14976",
    "vms",
    false,                      // whether binary
    true,                       // whether has multi-blocks
    &VamasDataSet::ctor,
    &VamasDataSet::check
);

} // namespace

namespace {

// NULL-terminated arrays -  dictionaries for VAMAS data

const char* exps[] = {
    "MAP","MAPDP","MAPSV","MAPSVDP","NORM",
    "SDP","SDPSV","SEM","NOEXP", NULL
};

const char* techs[] = {
    "AES diff","AES dir","EDX","ELS","FABMS",
    "FABMS energy spec","ISS","SIMS","SIMS energy spec","SNMS",
    "SNMS energy spec","UPS","XPS","XRF", NULL
};

const char* scans[] = {
    "REGULAR","IRREGULAR","MAPPING", NULL
};

// simple utilities

int read_line_int(istream& is)
{
    return my_strtol(read_line(is));
}

string read_line_trim(istream& is)
{
    return str_trim(read_line(is));
}

void assert_in_array(string const& val, const char** array, string const& name)
{
    for (const char** i = &array[0]; *i != NULL; ++i)
        if (strcmp(val.c_str(), *i) == 0)
            return;
    throw xylib::FormatError(name + "has an invalid value");
}

// skip "count" lines in f
void skip_lines(istream &f, int count)
{
    using namespace xylib;
    string line;
    for (int i = 0; i < count; ++i) {
        if (!getline(f, line)) {
            throw FormatError("unexpected end of file");
        }
    }
}


} // anonymous namespace


namespace xylib {


bool VamasDataSet::check(istream &f, string*)
{
    static const string magic =
     "VAMAS Surface Chemical Analysis Standard Data Transfer Format 1988 May 4";
    string line;
    skip_whitespace(f);
    return getline(f, line) && str_trim(line) == magic;
}


// Vamas file is organized as
// file_header block_header block_data [block_header block_data] ...
// There is only a value in every line without a key or label; the meaning is
// determined by its position and values in preceding lines
void VamasDataSet::load_data(std::istream &f, const char*)
{
    int n;
    skip_whitespace(f);
    skip_lines(f, 1);   // magic number: "VAMAS Sur..."
    meta["institution identifier"] = read_line_trim(f);
    meta["institution model identifier"] = read_line_trim(f);
    meta["operator identifier"] = read_line_trim(f);
    meta["experiment identifier"] = read_line_trim(f);

    // skip comment lines, n is number of lines
    n = read_line_int(f);
    skip_lines(f, n);

    exp_mode_ = read_line_trim(f);
    // make sure exp_mode_ has a valid value
    assert_in_array(exp_mode_, exps, "exp_mode");

    scan_mode_ = read_line_trim(f);
    // make sure scan_mode_ has a valid value
    assert_in_array(scan_mode_, scans, "scan_mode");

    // some exp_mode_ specific file-scope meta-info
    if ("MAP" == exp_mode_ || "MAPD" == exp_mode_ ||
        "NORM" == exp_mode_ || "SDP" == exp_mode_) {
        meta["number of spectral regions"] = read_line_trim(f);
    }
    if ("MAP" == exp_mode_ || "MAPD" == exp_mode_) {
        meta["number of analysis positions"] = read_line_trim(f);
        meta["number of discrete x coordinates available in full map"]
                                                            = read_line_trim(f);
        meta["number of discrete y coordinates available in full map"]
                                                            = read_line_trim(f);
    }

    // experimental variables
    exp_var_cnt_ = read_line_int(f);
    for (int i = 1; i <= exp_var_cnt_; ++i) {
        meta["experimental variable label " + S(i)] = read_line_trim(f);
        meta["experimental variable unit " + S(i)] = read_line_trim(f);
    }

    // number of entries in parameter inclusion or exclusion list
    n = read_line_int(f);
    // If the values of any of the block parameters are the same in all of the
    // blocks their values may be sent in the first block and then omitted
    // from all subsequent blocks.
    // n > 0 : the parameters listed are to be included
    // n < 0 : the parameters listed are to be excluded
    // n = 0 : all parameters are to be given in all blocks
    // A complete block contains 40 parts.
    bool all[40], inclusion_list[40];
    for (int i = 0; i < 40; ++i) {
        all[i] = true;
        inclusion_list[i] = (n <= 0);
    }
    for (int i = 0; i < (n >= 0 ? n : -n); ++i) {
        int idx = read_line_int(f) - 1; // "-1" because the input is 1-based
        inclusion_list[idx] = !inclusion_list[idx];
    }

    // # of manually entered items in block
    n = read_line_int(f);
    skip_lines(f, n);

    int exp_fue = read_line_int(f); // # of future upgrade experiment entries
    blk_fue_ = read_line_int(f); // # of future upgrade block entries
    skip_lines(f, exp_fue);

    // handle the blocks
    unsigned blk_cnt = read_line_int(f);
    const Block* first_block = NULL;
    for (unsigned i = 0; i < blk_cnt; ++i) {
        Block *blk = read_block(f, i == 0 ? all : inclusion_list, first_block);
        if (i == 0)
            first_block = blk;
        add_block(blk);
    }
}

static string two_digit(const string& s)
{
    if (s.empty())
        return "  ";
    return s.length() == 1 ? "0"+s : s;
}

// read one block from file
Block* VamasDataSet::read_block(istream &f, bool includes[],
                                const Block* first_block)
{
    Block *block = new Block;
    double x_start=0., x_step=0.;
    string x_name;

    vector<VecColumn*> ycols;

    block->set_name(read_line_trim(f));
    block->meta["sample identifier"] = read_line_trim(f);

    string date_time;
    string first_dt;
    if (first_block)
        first_dt = first_block->meta.get("date_time");
    // year, month, etc. should be numbers, but don't assume it, just in case
    if (includes[0]) {
        date_time = read_line_trim(f);
        if (date_time.size() < 4)
            date_time.insert(date_time.begin(), 4 - date_time.size(), ' ');
    } else {
        date_time = first_dt.substr(0, 4);
    }

    date_time += includes[1] ? "-" + two_digit(read_line_trim(f))
                             : first_dt.substr(4, 3);;
    date_time += includes[2] ? "-" + two_digit(read_line_trim(f))
                             : first_dt.substr(7, 3);;
    date_time += includes[3] ? " " + two_digit(read_line_trim(f))
                             : first_dt.substr(10, 3);;
    date_time += includes[4] ? ":" + two_digit(read_line_trim(f))
                             : first_dt.substr(13, 3);;
    date_time += includes[5] ? ":" + two_digit(read_line_trim(f))
                             : first_dt.substr(16, 3);;
    if (includes[6]) {
        string timezone = read_line_trim(f);
        if (!timezone.empty()) {
            if (timezone[0] == '-')
                date_time += " -" + two_digit(timezone.substr(1)) + "00";
            else
                date_time += " +" + two_digit(timezone) + "00";
        }
    }
    block->meta["date_time"] = date_time;

    if (includes[7]) {   // skip comments on this block
        int cmt_lines = read_line_int(f);
        skip_lines(f, cmt_lines);
    }

    string tech;
    if (includes[8]) {
        tech = read_line_trim(f);
        block->meta["tech"] = tech;
        assert_in_array(tech, techs, "tech");
    }

    if (includes[9]) {
        if ("MAP" == exp_mode_ || "MAPDP" == exp_mode_) {
            block->meta["x coordinate"] = read_line_trim(f);
            block->meta["y coordinate"] = read_line_trim(f);
        }
    }

    if (includes[10]) {
        for (int i = 0; i < exp_var_cnt_; ++i) {
            block->meta["experimental variable value " + S(i)] = read_line_trim(f);
        }
    }

    if (includes[11])
        block->meta["analysis source label"] = read_line_trim(f);

    if (includes[12]) {
        if ("MAPDP" == exp_mode_ || "MAPSVDP" == exp_mode_
                || "SDP" == exp_mode_ || "SDPSV" == exp_mode_
                || "SNMS energy spec" == tech || "FABMS" == tech
                || "FABMS energy spec" == tech || "ISS" == tech
                || "SIMS" == tech || "SIMS energy spec" == tech
                || "SNMS" == tech) {
            block->meta["sputtering ion oratom atomic number"]
                                                          = read_line_trim(f);
            block->meta["number of atoms in sputtering ion or atom particle"]
                                                          = read_line_trim(f);
            block->meta["sputtering ion or atom charge sign and number"]
                                                          = read_line_trim(f);
        }
    }

    if (includes[13])
        // a.k.a "analysis source characteristic energy"
        block->meta["source energy"] = read_line_trim(f);
    if (includes[14])
        block->meta["analysis source strength"] = read_line_trim(f);

    if (includes[15]) {
        block->meta["analysis source beam width x"] = read_line_trim(f);
        block->meta["analysis source beam width y"] = read_line_trim(f);
    }

    if (includes[16]) {
        if ("MAP" == exp_mode_ || "MAPDP" == exp_mode_ || "MAPSV" == exp_mode_
                || "MAPSVDP" == exp_mode_ || "SEM" == exp_mode_) {
            block->meta["field of view x"] = read_line_trim(f);
            block->meta["field of view y"] = read_line_trim(f);
        }
    }

    if (includes[17]) {
        if ("SEM" == exp_mode_ || "MAPSV" == exp_mode_
                || "MAPSVDP" == exp_mode_) {
            throw FormatError("unsupported MAPPING mode");
        }
    }

    if (includes[18])
        block->meta["analysis source polar angle of incidence"] = read_line_trim(f);
    if (includes[19])
        block->meta["analysis source azimuth"] = read_line_trim(f);
    if (includes[20])
        block->meta["analyser mode"] = read_line_trim(f);
    if (includes[21])
        block->meta["analyser pass energy or retard ratio or mass resolution"]
                                                            = read_line_trim(f);
    if (includes[22]) {
        if ("AES diff" == tech) {
            block->meta["differential width"] = read_line_trim(f);
        }
    }

    if (includes[23])
        block->meta["magnification of analyser transfer lens"] = read_line_trim(f);
    if (includes[24])
        block->meta["analyser work function or acceptance energy of atom or ion"] = read_line_trim(f);
    if (includes[25])
        block->meta["target bias"] = read_line_trim(f);

    if (includes[26]) {
        block->meta["analysis width x"] = read_line_trim(f);
        block->meta["analysis width y"] = read_line_trim(f);
    }

    if (includes[27]) {
        block->meta["analyser axis take off polar angle"] = read_line_trim(f);
        block->meta["analyser axis take off azimuth"] = read_line_trim(f);
    }

    if (includes[28])
        block->meta["species label"] = read_line_trim(f);

    if (includes[29]) {
        block->meta["transition or charge state label"] = read_line_trim(f);
        block->meta["charge of detected particle"] = read_line_trim(f);
    }

    if (includes[30]) {
        if ("REGULAR" == scan_mode_) {
            x_name = read_line_trim(f);
            block->meta["abscissa label"] = x_name;
            block->meta["abscissa units"] = read_line_trim(f);
            x_start = my_strtod(read_line(f));
            x_step = my_strtod(read_line(f));
        }
        else
            throw FormatError("Only REGULAR scans are supported now");
    }
    else
        throw FormatError("how to find abscissa properties in this file?");

    int cor_var; // number of corresponding variables
    if (includes[31]) {
        cor_var = read_line_int(f);
        if (cor_var < 1)
            throw FormatError("wrong number of corresponding variables");
        // columns initialization
        for (int i = 0; i != cor_var; ++i) {
            string corresponding_variable_label = read_line_trim(f);
            skip_lines(f, 1);    // ignoring corresponding variable unit
            ycols.push_back(new VecColumn);
            ycols[i]->set_name(corresponding_variable_label);
        }
    } else {
        assert(first_block != NULL);
        cor_var = first_block->get_column_count() - 1; // don't count xcol
        for (int i = 0; i != cor_var; ++i) {
            ycols.push_back(new VecColumn);
            ycols[i]->set_name(first_block->get_column(i).get_name());
        }
    }

    if (includes[32])
        block->meta["signal mode"] = read_line_trim(f);
    if (includes[33])
        block->meta["signal collection time"] = read_line_trim(f);
    if (includes[34])
        block->meta["# of scans to compile this blk"] = read_line_trim(f);
    if (includes[35])
        block->meta["signal time correction"] = read_line_trim(f);

    if (includes[36]) {
        if (("AES diff" == tech || "AES dir" == tech || "EDX" == tech ||
             "ELS" == tech || "UPS" == tech || "XPS" == tech || "XRF" == tech)
            && ("MAPDP" == exp_mode_ || "MAPSVDP" == exp_mode_
                || "SDP" == exp_mode_ || "SDPSV" == exp_mode_)) {
            skip_lines(f, 7);
        }
    }

    if (includes[37]) {
        block->meta["sample normal polar angle of tilt"] = read_line_trim(f);
        block->meta["sample normal polar tilt azimuth"] = read_line_trim(f);
    }

    if (includes[38])
        block->meta["sample rotate angle"] = read_line_trim(f);

    if (includes[39]) {
        int n = read_line_int(f);   // # of additional numeric parameters
        for (int i = 0; i < n; ++i) {
            // 3 items in every loop: param_label, param_unit, param_value
            string param_label = read_line_trim(f);
            string param_unit = read_line_trim(f);
            block->meta[param_label] = read_line_trim(f) + param_unit;
        }
    }

    skip_lines(f, blk_fue_); // skip future upgrade block entries

    int cur_blk_steps = read_line_int(f);
    skip_lines(f, 2 * cor_var);   // min & max ordinate

    StepColumn *xcol = new StepColumn(x_start, x_step);
    xcol->set_name(x_name);
    block->add_column(xcol);

    int col = 0;
    assert(ycols.size() == (size_t) cor_var);
    for (int i = 0; i < cur_blk_steps; ++i) {
        double y = my_strtod(read_line_trim(f));
        ycols[col]->add_val(y);
        col = (col + 1) % cor_var;
    }
    for (int i = 0; i < cor_var; ++i)
        block->add_column(ycols[i]);
    return block;
}

} // namespace xylib

