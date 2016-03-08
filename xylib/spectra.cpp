// Ron Unwin's Spectra data file
// Licence: Lesser GNU Public License 2.1 (LGPL)
// Author: Matthias Richter


#define BUILDING_XYLIB
#include "spectra.h"

#include <cstdlib>
#include "util.h"

using namespace std;
using namespace xylib::util;

namespace xylib {

const FormatInfo SpectraDataSet::fmt_info(
    "spectra",
    "Spectra / VGX 900",
    "1 2 3 4 5 6 7 8 9",    // file endings are numbered, number is increased
                            // for each new data aquisition in the same project
    false,                  // whether binary
    true,                   // whether has multi-blocks
    &SpectraDataSet::ctor,
    &SpectraDataSet::check
);

bool SpectraDataSet::check(istream &f, string*)
{
    f.ignore(1024, '\n'); // first line should be experimentname
    char line[256];
    f.getline(line, 255); // second line should be parameters
    if (!f || f.gcount() > 200 || count_numbers(line) != 8)
        return false;
    f.ignore(1024, '\n'); // third line should be spectrumname
    // check that the next few lines have only single integer number
    for (int i = 0; i != 3; ++i) {
        f.getline(line, 32);
        if (!f || f.gcount() > 30)
            return false;
        char *endptr;
        (void) strtol(line, &endptr, 10);
        if (endptr == line)
            return false;
        while (isspace(*endptr))
            ++endptr;
        if (*endptr != '\0')
            return false;
    }
    return true;
}


void SpectraDataSet::load_data(std::istream &f)
{
    f.ignore(1024, '\n'); // first line --> Experimentname
    for (;;) {
        Block *blk = read_block(f);
        if (blk == NULL)
            break;
        add_block(blk);
    }
}

Block* SpectraDataSet::read_block(istream& f)
{
    char line[256];
    f.getline(line, 255); // second line --> header
    if (f.eof())
        return NULL;
    const char *pStart = line;
    char *pEnd;
    double start = strtod(pStart,&pEnd);
    format_assert(this, pEnd != pStart);
    pStart = pEnd;
    double end = strtod(pStart,&pEnd);
    pStart = pEnd;
    double step = strtod(pStart,&pEnd);
    pStart = pEnd;
    double scans = strtod(pStart,&pEnd);
    pStart = pEnd;
    double dwell = strtod(pStart,&pEnd);
    pStart = pEnd;
    // supposedly it's always integer, but using strtod just in case
    long points = (long) strtod(pStart,&pEnd);
    format_assert(this, pEnd != pStart);
    format_assert(this, points > 0 && points < 1e8,
                  "unexpected 6th parameter (#points)");
    pStart = pEnd;
    double epass = strtod(pStart,&pEnd);
    pStart = pEnd;
    double exenergy = strtod(pStart,&pEnd);
    format_assert(this, pEnd != pStart);

    f.getline(line, 255); // third line --> spectraname
    format_assert(this, !f.eof(), "unexpected EOF");
    // It's a file from a DOS program. According to the manual it's
    // ASCII file, but some files have non-ASCII characters.
    // Let's "convert" them to ascii.
    for (char *p = line; *p != '\0'; ++p)
        if (*p == (char) 0xB0) // some files have degree symbol in Latin1
            *p = '^';
        else if ((signed char) *p < 0)
            *p = '?';
    string spectra_name = str_trim(line);

    Block *blk = new Block;
    blk->set_name(spectra_name);
    blk->meta["start"]          = dbl_to_str(start);
    blk->meta["end"]            = dbl_to_str(end);
    blk->meta["step"]           = dbl_to_str(step);
    blk->meta["scans"]          = dbl_to_str(scans);
    blk->meta["dwell"]          = dbl_to_str(dwell);
    blk->meta["points"]         = S(points);
    blk->meta["EPass"]          = dbl_to_str(epass);
    blk->meta["source energy"]  = dbl_to_str(exenergy);

    // positive binding energy
    //StepColumn *xcol = new StepColumn(exenergy-start, -step);
    // negative binding energy
    StepColumn *xcol = new StepColumn(start-exenergy, step);
    xcol->set_name("binding energy [eV]");
    blk->add_column(xcol);

    VecColumn *ycol = new VecColumn;
    ycol->set_name(spectra_name + " [cps]");
    for (long i = 0; i != points; ++i) {
        f.getline(line, 32);
        format_assert(this, !f.fail(), "reading cps data failed");
        ycol->add_val(my_strtod(line)/scans/dwell);
    }
    blk->add_column(ycol);
    return blk;
}

} // namespace xylib

