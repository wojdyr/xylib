// Bruker ESP300-E raw binary spectrum
// Licence: Lesser GNU Public License 2.1 (LGPL)
// Implementation based on work by Christoph Burow done for his R package 'ESR'
// https://github.com/tzerk/ESR
// Implementation carried out by Sebastian Kreutzer, IRAMAT-CRP2A, Universite Bordeaux Montaigne, France

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

bool BrukerSpcDataSet::check(std::istream &f, string*)
{

  return true;
}


void BrukerSpcDataSet::load_data(std::istream &f)
{


  Block* blk = new Block;

  //predefine vectors
  VecColumn *xcol = new VecColumn;
  VecColumn *ycol = new VecColumn;

  //initialise values
  int channel = 1;
  int stop = 0;

  //run the import until we struggle
  while (stop == 0) {

    //we only try
    try{

      //the file format is quite simple, however,
      //we have BIG endian, means we have to swap the byte position ...
      int y = swap_int32(read_int32_le(f));
      ycol -> add_val(y);

      //set x-value and update channel number
      xcol -> add_val(channel);
      channel++;

    }

    //catch the expection and set stop to 1
    catch (const exception& e) {
      stop = 1;
    }

  }

  //add block data
  blk->add_column(xcol);
  blk->add_column(ycol);
  add_block(blk);

}

} // end of namespace xylib

