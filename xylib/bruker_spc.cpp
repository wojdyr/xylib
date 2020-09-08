// Bruker ESP300-E raw binary spectrum
// Licence: Lesser GNU Public License 2.1 (LGPL)
// Implementation based on work by Christoph Burow done for his R package 'ESR'
// https://github.com/tzerk/ESR
// Implementation carried out by Sebastian Kreutzer, IRAMAT-CRP2A,
// Universite Bordeaux Montaigne, France

#define BUILDING_XYLIB
#include "bruker_spc.h"
#include <sstream>
#include "util.h"

using namespace xylib::util;

namespace xylib {

const FormatInfo BrukerSpcDataSet::fmt_info(
    "bruker_spc",
    "Bruker ESP300-E SPC",
    "spc",
    true,                        // whether binary
    false,                       // whether has multi-blocks
    &BrukerSpcDataSet::ctor,
    &BrukerSpcDataSet::check
);

bool BrukerSpcDataSet::check(std::istream&, std::string*)
{
    return true;
}

void BrukerSpcDataSet::load_data(std::istream &f, const char* path)
{
    // (1) read y-data from the SPC-file
    VecColumn *ycol = new VecColumn;

    int i = 0;
    while (i == 0) {
        try { // read until read_int32_be throws error
            int y = read_int32_be(f);
            ycol->add_val(y);

        } catch (const FormatError& e) {
             break;
        }
    }

    Block* blk = new Block;
    blk->add_column(new StepColumn(1, 1));
    blk->add_column(ycol);

    add_block(blk);

    // (2) read meta-data from the PAR-file if available
    // the PAR-file should be in the same folder, with the same basename
    // and extension either PAR or par.
    std::string par = path;
    if (par.length() < 4)
        return;
    par.replace(par.end()-3, par.end(), "PAR");
    std::ifstream par_file(par.c_str());
#ifndef _WIN32  // Windows is case-insensitive, no need to check for *.par
    if (!par_file) {
        par.replace(par.end()-3, par.end(), "par");
        par_file.open(par.c_str());
    }
#endif
    if (par_file) {
        std::string line;
        while (std::getline(par_file, line, '\r')) {
            std::string key, value;
            str_split(line, " ", key, value);
            if (value.find('\n') == std::string::npos)
                meta[key] = value;
        }
    }
}

} // end of namespace xylib
