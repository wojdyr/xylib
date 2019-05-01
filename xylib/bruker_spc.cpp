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

using namespace std;
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

bool BrukerSpcDataSet::check(std::istream&, string*)
{
    return true;
}

void BrukerSpcDataSet::load_data(std::istream &f, const char*)
{
    VecColumn *ycol = new VecColumn;
    try { // read until read_int32_be throws error
        int y = read_int32_be(f);
        ycol->add_val(y);
    }
    catch (const FormatError& e) {}
    Block* blk = new Block;
    blk->add_column(new StepColumn(1, 1));
    blk->add_column(ycol);
    add_block(blk);
}

} // end of namespace xylib

