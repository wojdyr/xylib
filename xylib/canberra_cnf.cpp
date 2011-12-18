// Canberra CNF (one of variants)
// Licence: Lesser GNU Public License 2.1 (LGPL)
// $Id: $

#define BUILDING_XYLIB
#include "canberra_cnf.h"

#include <cmath>
#include <boost/cstdint.hpp>

#include "util.h"

using namespace std;
using namespace xylib::util;
using boost::uint16_t;
using boost::uint32_t;


namespace xylib {


const FormatInfo CanberraCnfDataSet::fmt_info(
    "canberra_cnf",
    "Canberra CNF",
    "cnf",
    true,                       // whether binary
    false,                      // whether has multi-blocks
    &CanberraCnfDataSet::ctor,
    &CanberraCnfDataSet::check
);

bool CanberraCnfDataSet::check(istream &f)
{
    const int size = 0x8c0;
    char *data = new char[size];
    f.read(data, size);
    // some cnf files have "PHA+" at 0x8b0 and number of channels at 0x8b9
    bool has_pha = (data[0x8b0] == 'P' && data[0x8b1] == 'H' &&
                    data[0x8b2] == 'A');
    delete [] data;
    return f.gcount() == size && has_pha;
}

void CanberraCnfDataSet::load_data(std::istream &f)
{
    const int max_size = 0x17DFC;
    char *data = new char[max_size];
    f.read(data, max_size);
    int size = f.gcount();
    if (size < 0x5870) {
        delete [] data;
        throw FormatError("Read error or unexpected end of file.");
    }

    // energy calibration is there, but where exactly?
    double energy_offset = 0;
    double energy_slope = 1.;
    double energy_quadr = 0.;

    uint16_t n_channels = *reinterpret_cast<uint16_t*>(data+0x8b9);
    le_to_host(&n_channels, 2);
    int data_offset = size - 4 * n_channels;
    if (data_offset < 0x5870) {
        delete [] data;
        throw FormatError("apparently we do not know how to read this file.");
    }

    Block* blk = new Block;

    Column *xcol = NULL;
    if (energy_quadr) {
        VecColumn *vc = new VecColumn;
        for (int i = 1; i <= n_channels; i++) {
            double x = energy_offset + energy_slope * i + energy_quadr * i * i;
            vc->add_val(x);
        }
        xcol = vc;
    }
    else {
        xcol = new StepColumn(energy_offset+energy_slope, energy_slope);
    }
    blk->add_column(xcol);

    VecColumn *ycol = new VecColumn;
    for (int i = 0; i < n_channels; i++) {
        uint32_t y = *reinterpret_cast<uint32_t*>(data+data_offset+4*i);
        le_to_host(&y, 4);
        ycol->add_val(y);
    }
    blk->add_column(ycol);

    //blk->set_name(str_trim(string(data+0x5830, 63)));
    blk->set_name(str_trim(string(data+0x5870, 16)));
    add_block(blk);
}

} // namespace xylib

