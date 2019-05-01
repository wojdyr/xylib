// SpecsLab2 XY data file
// Licence: Lesser GNU Public License 2.1 (LGPL)

#define BUILDING_XYLIB
#include "specsxy.h"

#include <cctype>
#include <cstring>
#include <string>
#include <vector>

#include "util.h"

using namespace std;
using namespace xylib::util;


namespace xylib {


const FormatInfo SpecsxyDataSet::fmt_info(
    "specsxy",
    "SPECS SpecsLab2 xy",
    "xy",
    false,                     // whether binary
    true,                      // whether has multi-blocks
    &SpecsxyDataSet::ctor,
    &SpecsxyDataSet::check
);

bool SpecsxyDataSet::check(istream &f, string*)
{
    char buffer[32];
    f.get(buffer, 32);
    return strcmp(buffer, "# Created by:        SpecsLab2,") == 0;
}

static
Block* read_block(istream &f)
{
    Block* blk = new Block;
    string line;
    // ColumnLabels are appropriate column lables, but the label "energy"
    // may stand for either KE or BE, we try to be more specific.
    string what_energy;
    // metadata
    while (getline(f, line)) {
        if (line.empty())
            continue;
        if (line[0] == '#') {
            string::size_type colon = line.find(':');
            if (colon == string::npos) {
                if (str_startwith(line, "# values in binding energy"))
                    what_energy = "binding ";
                else if (str_startwith(line, "# values in kinetic energy"))
                    what_energy = "kinetic ";
            } else if (line[1] == ' ' && isupper(line[2])) { // ^# [A-Z].*:.*$
                string key = str_trim(line.substr(1, colon-1));
                string value = str_trim(line.substr(colon+1));
                if (key == "RemoteInfo" || key == "Parameter" ||
                    key == "XY-Serializer Export Settings" || value.empty()) {
                    // ignore, these fields are non-unique or not interesting
                } else if (key == "Excitation Energy") {
                    blk->meta["source energy"] = value;
                } else { // all other 
                    blk->meta[key] = value;
                }
            }
        } else if (line.find_first_of("0123456789") < line.find('#')) {
            break;  // first data line was read but not parsed yet
        }
    }
    if (blk->meta.has_key("Region")) {
        string title = blk->meta.get("Region");
        if (blk->meta.has_key("Acquisition Date"))
            title += " (" + blk->meta.get("Acquisition Date") + ")";
        blk->set_name(title);
    }
    // data - first line
    vector<double> row;
    read_numbers(line, row);
    if (row.empty()) {
        delete blk;
        return NULL;
    }
    vector<VecColumn*> cols;
    cols.reserve(row.size());
    for (size_t i = 0; i != row.size(); ++i) {
        cols.push_back(new VecColumn);
        cols[i]->add_val(row[i]);
        blk->add_column(cols[i]);
    }
    if (blk->meta.has_key("ColumnLabels")) {
        const string& labels = blk->meta.get("ColumnLabels");
        string::size_type start_pos = 0, pos=0;
        for (size_t i = 0; i != cols.size() && pos != string::npos; ++i) {
            pos = labels.find_first_of(" \t", start_pos);
            string label = labels.substr(start_pos, pos-start_pos);
            cols[i]->set_name(label == "energy" ? what_energy + label : label);
            start_pos = pos+1;
        }
    }
    // data - next lines  (data block ends with blank line or eof)
    while (getline(f, line) && !line.empty() && line[0] != '#') {
        read_numbers(line, row);
        if (row.size() == 0)
            break;
        if (row.size() != cols.size()) {
            warn("Warning. Expected %d numbers in line, got %d.\n",
                 (int) cols.size(), (int) row.size());
            row.resize(cols.size(), 0);
        }
        for (size_t i = 0; i != row.size(); ++i)
            cols[i]->add_val(row[i]);
    }
    return blk;
}

void SpecsxyDataSet::load_data(std::istream &f, const char*)
{
    Block* blk = NULL;
    while ((blk = read_block(f)) != NULL)
        add_block(blk);
}

} // namespace xylib

