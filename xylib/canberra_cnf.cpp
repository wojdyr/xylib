// Canberra CNF (aka CAM format) with spectral data from Genie software
// Licence: Lesser GNU Public License 2.1 (LGPL)

#define BUILDING_XYLIB
#include "canberra_cnf.h"

#include <ctime>
#include <cstring>
#include <boost/cstdint.hpp>

#include "util.h"

using namespace std;
using namespace xylib::util;
using boost::uint16_t;
using boost::uint32_t;
using boost::uint64_t;


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

bool CanberraCnfDataSet::check(istream &f, string*)
{
    int acq_offset = 0;
    f.ignore(112);
    int pos = 112;
    char buf[48];
    while (!f.eof()) {
        f.read(buf, 48);
        if (f.gcount() != 48)
            return false;
        if ((buf[1] != 0x20 || buf[2] != 0x01) && buf[1] != 0 && buf[2] != 0)
            return false;
        pos += 48;
        if (buf[0] == 0) {
            acq_offset = from_le<uint32_t>(buf+10);
            break;
        }
    }
    if (acq_offset <= pos)
        return false;
    f.ignore(acq_offset - pos);
    f.read(buf, 48);
    return (!f.eof() && f.gcount() == 48 && buf[0] == 0 && buf[1] == 0x20);
}

static
Column* read_energy_callibration(const char* p, Block *blk, int n_channels)
{
    // energy calibration
    p += 2*4+28;
    double coef[3];
    for (int i = 0; i != 3; ++i)
        coef[i] = from_pdp11((unsigned char*) p + 4*i);
    if (coef[1] == 0.)
        return NULL;
    for (int i = 0; i != 3; ++i)
        blk->meta["energy calib "+S(i)] = format1<double,16>("%.7g", coef[i]);
    if (coef[2] != 0.) { // quadr term
        VecColumn *vc = new VecColumn;
        // Comparing results with FitzPeaks and and Cambio 4.0
        // the first channel should have number 1 (not 0).
        for (int i = 1; i <= n_channels; i++) {
            double x = coef[0] + coef[1] * i + coef[2] * i * i;
            vc->add_val(x);
        }
        return vc;
    }
    else
        // since we start from ch1, the first value is coef[0] + coef[1]
        return new StepColumn(coef[0]+coef[1], coef[1]);
}

static
bool is_printable(const string &s)
{
    for (size_t i = 0; i != s.size(); ++i)
        if (!isprint(s[i]))
                return false;
    return true;
}

// comment from Stefan Schneider-Kennedy:
// > 8 byte little endian. Number of 100ns intervals since 1859.
// > Like Microsoft's FILETIME with an offset.
//
// comment from Stephan Messlinger:
// > The magic offset of 3506716800 seconds between the CNF time format to
// > unixtime does not point to 1859 but to Nov 17, 1958, 00:00. This is the
// > starting point for the 'Modified Julian Date', which is commonly used in
// > astronomy. Together with resolution of 100 ns, this points out to the DEC
// > VMS time format (which again fits to the PDP11 floating point format for
// > the energy coefficients).
//
static
string convert_date(const char* p)
{
    uint64_t d;
    memcpy(&d, p, sizeof(d));
    le_to_host(&d, sizeof(d));
    time_t t = d / 10000000 - 3506716800u; // time since the Epoch
    char s[64];
    int r = strftime(s, sizeof(s), "%a, %Y-%m-%d %H:%M:%S", gmtime(&t));
    if (r == 0)
        throw FormatError("reading date failed.");
    return string(s);
}

// comment from Stefan Schneider-Kennedy:
// > 8 byte little endian. 2^(64) - number of 100ns intervals since timing
// > started. Like Microsoft's FILETIME. Round to nearest second.
static
float convert_time(const char* p)
{
    uint64_t d;
    memcpy(&d, p, sizeof(d));
    le_to_host(&d, sizeof(d));
    return (~d) * 1.0e-7f;
}


void CanberraCnfDataSet::load_data(std::istream &f)
{
    string file_string;
    file_string.reserve(128*1024);
    file_string.assign((istreambuf_iterator<char>(f)),
                       istreambuf_iterator<char>());
    const char* beg = file_string.c_str();
    const char* end = beg + file_string.size();

    int acq_offset = 0, sam_offset = 0, eff_offset = 0, enc_offset = 0,
        chan_offset = 0;

    // we have here 48-byte blocks with meta data
    for (const char* ptr = beg+112; ptr+48 < end; ptr += 48) {
        // MW - should records 0 in ptr[1] or ptr[2] be really accepted?
        if ((ptr[1] != 0x20 || ptr[2] != 0x01) && ptr[1] != 0 && ptr[2] != 0)
            break;
        uint32_t offset = from_le<uint32_t>(ptr+10);
#if 0
        printf("%6ld: hex %02x %02x %02x %02x  -> %6u",
               ptr-beg, ptr[0], ptr[1], ptr[2], ptr[3], offset);
        if (offset > 0 && beg+offset+1 < end)
            printf(" -> hex %02x %02x", beg[offset], beg[offset+1]);
        printf("\n");
#endif
        switch (ptr[0]) {
            case 0:
                if (acq_offset == 0)
                    acq_offset = offset;
                else
                    enc_offset = offset;
                break;
            case 1:
                if (sam_offset == 0)
                    sam_offset = offset;
                break;
            case 2:
                if (eff_offset == 0)
                    eff_offset = offset;
                break;
            case 5:
                // We don't have specs, so it's all based on guessing.
                // At least for me. Checking a few CNF files I've got,
                // normally we are here when the record starts with 0x052001,
                // but the condition above (which I copied like everything
                // else from JF) is also passing when it starts with, say,
                // 0x054000 (Miha Cernetic sent us such a file).
                // No idea if the condition above can be safely changed.
                // Here is a workaround from JF - checking the value at the
                // offset.
                if (chan_offset == 0 &&
                           offset+1 < file_string.size() &&
                           file_string[offset] == '\x05' &&
                           file_string[offset+1] == '\x20')
                       chan_offset = offset;
                break;
            default:
                break;
        }
        if (acq_offset != 0 && sam_offset != 0 &&
                    eff_offset != 0 && chan_offset != 0)
            break;
    }
    if (enc_offset == 0)
        enc_offset = acq_offset;

    AutoPtrBlock blk(new Block);

    // sample data - split name into name and description
    // (this was not in code from JF, it's my guess - MW)
    const char* sam_ptr = beg + sam_offset;
    if (sam_ptr+0x30+80 >= end || sam_ptr[0] != 1 || sam_ptr[1] != 0x20)
        warn("Warning. Sample data not found.\n");
    else {
        string name = str_trim(string(sam_ptr+0x30, 32));
        if (!name.empty() && is_printable(name))
            blk->set_name(name);
        string desc = str_trim(string(sam_ptr+0x30+32, 48));
        if (!desc.empty() && is_printable(desc))
            blk->meta["description"] = desc;
    }

    // go to acquisition data
    const char* acq_ptr = beg + acq_offset;
    if (acq_ptr+48+128+10+4 >= end || acq_ptr[0] != 0 || acq_ptr[1] != 0x20)
        throw FormatError("Acquisition data not found.");
    uint16_t offset1 = from_le<uint16_t>(acq_ptr+34);
    uint16_t offset2 = from_le<uint16_t>(acq_ptr+36);

    const char* pha_ptr = acq_ptr + 48 + 128;
    if (pha_ptr[0] != 'P' || pha_ptr[1] != 'H' || pha_ptr[2] != 'A')
        warn("Warning. PHA keyword not found.\n");

    int n_channels = from_le<uint16_t>(pha_ptr+10) * 256;
    if (n_channels < 256 || n_channels > 16384)
        throw FormatError("Unexpected number of channels" + S(n_channels));

    // dates and times
    const char* date_ptr = acq_ptr+48+offset2+1;
    format_assert(this, date_ptr + 3*8 < end);
    blk->meta["date and time"] = convert_date(date_ptr); // when taken
    float real_time = convert_time(date_ptr + 8);
    blk->meta["real time (s)"] = format1<float, 16>("%.2f", real_time);
    float live_time = convert_time(date_ptr + 16);
    blk->meta["live time (s)"] = format1<float, 16>("%.2f", live_time);

    format_assert(this, beg + enc_offset+48+32 + offset1 < end);
    Column *xcol = read_energy_callibration(beg + enc_offset+48+32 + offset1,
                                            blk.get(), n_channels);
    if (xcol == NULL)
        xcol = read_energy_callibration(beg + enc_offset+48+32,
                                        blk.get(), n_channels);
    if (xcol == NULL) {
        warn("Warning. Energy Calibration not found.\n");
        xcol = new StepColumn(1, 1);
    }

    // code from JF is also reading detector name, but it's not needed here
    // detector name was 232 bytes after energy calibration

    // channel data
    const char* chan_ptr = beg + chan_offset;
    if (chan_ptr+512+4*n_channels > end || chan_ptr[0] != 5 ||
                                           chan_ptr[1] != 0x20) {
        delete xcol;
        throw FormatError("Channel data not found.");
    }
    VecColumn *ycol = new VecColumn;
    // the two first channels sometimes contain live and real time
    for (int i = 0; i < 2; ++i) {
        uint32_t y = from_le<uint32_t>(chan_ptr+512+4*i);
        if ((int) y == iround(real_time) || (int) y == iround(live_time))
            y = 0;
        ycol->add_val(y);
    }
    for (int i = 2; i < n_channels; ++i) {
        uint32_t y = from_le<uint32_t>(chan_ptr+512+4*i);
        ycol->add_val(y);
    }

    blk->add_column(xcol);
    blk->add_column(ycol);
    add_block(blk.release());
}

} // namespace xylib

