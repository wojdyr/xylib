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
    "spc par",
    true,                        // whether binary
    false,                       // whether has multi-blocks
    &BrukerSpcDataSet::ctor,
    &BrukerSpcDataSet::check

);

bool BrukerSpcDataSet::check(std::istream&, string*)
{
    return true;
}

void BrukerSpcDataSet::load_data(std::istream &f, const char* path)
{
    //(1) read y-data from the SPC-file
    VecColumn *ycol = new VecColumn;
    try { // read until read_int32_be throws error
        int y = read_int32_be(f);
        ycol->add_val(y);
    }

    catch (const FormatError& e) {}
    Block* blk = new Block;
    blk->add_column(new StepColumn(1, 1));
    blk->add_column(ycol);

    //add block
    add_block(blk);

    //(2) read meta-data from the PAR-file if available
    //the PAR-file is a second file that should be in the
    //same folder
    string par = path;

    if(par.length() > 3){
     string key, value;

        try {
         ifstream par_file(par.replace(par.end()-3, par.end(), "PAR"));

             for(std::string line; getline(par_file, line);){
               par_file >> key >> value;
               meta[key] = value;

             }

        }

        catch (const FormatError& e) {}

    }

}

} // end of namespace xylib

