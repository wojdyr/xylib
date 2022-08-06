// Siemens/Bruker Diffrac-AT Raw Format version 1/2/3
// Licence: Lesser GNU Public License 2.1 (LGPL)

#define BUILDING_XYLIB
#include "bruker_raw.h"
#include <sstream>
#include "util.h"

using namespace std;
using namespace xylib::util;

namespace xylib {

const FormatInfo BrukerRawDataSet::fmt_info(
    "bruker_raw",
    "Siemens/Bruker RAW",
    "raw",
    true,                       // whether binary
    true,                       // whether has multi-blocks
    &BrukerRawDataSet::ctor,
    &BrukerRawDataSet::check
);


bool BrukerRawDataSet::check(istream &f, string* details)
{
    string head = read_string(f, 4);
    if (head == "RAW ") {
        if (details)
            *details = "ver. 1";
        return true;
    }
    else if (head == "RAW2") {
        if (details)
            *details = "ver. 2";
        return true;
    }
    else if (head == "RAW1" && read_string(f, 3) == ".01") {
        if (details)
            *details = "ver. 3";
        return true;
    }
    else if (head == "RAW4" && read_string(f, 3) == ".00") {
        if (details)
            *details = "ver. 4";
        return true;
    }
    else
        return false;
}


void BrukerRawDataSet::load_data(std::istream &f, const char*)
{
    string head = read_string(f, 4);
    format_assert(this, head == "RAW " || head == "RAW2" || head == "RAW1" || head == "RAW4");
    if (head[3] == ' ')
        load_version1(f);
    else if (head[3] == '2')
        load_version2(f);
    else if (head[3] == '1')
        load_version1_01(f);
    else // head[3] == '4'
        load_version4(f);
}

void BrukerRawDataSet::load_version1(std::istream &f)
{
    meta["format version"] = "1";

    unsigned following_range = 1;

    while (following_range > 0) {
        Block* blk = new Block;

        unsigned cur_range_steps = read_uint32_le(f);
        // early DIFFRAC-AT raw data files didn't repeat the "RAW "
        // on additional ranges
        // (and if it's the first block, 4 bytes from file were already read)
        if (get_block_count() != 0) {
            istringstream raw_stream("RAW ");
            unsigned raw_int = read_uint32_le(raw_stream);
            if (cur_range_steps == raw_int)
                cur_range_steps = read_uint32_le(f);
        }

        blk->meta["MEASUREMENT_TIME_PER_STEP"] = S(read_flt_le(f));
        float x_step = read_flt_le(f);
        blk->meta["SCAN_MODE"] = Su(read_uint32_le(f));
        f.ignore(4);
        float x_start = read_flt_le(f);

        StepColumn *xcol = new StepColumn(x_start, x_step);
        blk->add_column(xcol);

        float t = read_flt_le(f);
        // documentation says: "-1.E6 = unknown"
        if (-1e6 != t)
            blk->meta["THETA_START"] = S(t);

        t = read_flt_le(f);
        if (-1e6 != t)
            blk->meta["KHI_START"] = S(t);

        t = read_flt_le(f);
        if (-1e6 != t)
            blk->meta["PHI_START"] = S(t);

        blk->meta["SAMPLE_NAME"] = read_string(f, 32);
        blk->meta["K_ALPHA1"] = S(read_flt_le(f));
        blk->meta["K_ALPHA2"] = S(read_flt_le(f));

        f.ignore(72);   // unused fields
        following_range = read_uint32_le(f);

        VecColumn *ycol = new VecColumn();

        for(unsigned i = 0; i < cur_range_steps; ++i) {
            float y = read_flt_le(f);
            ycol->add_val(y);
        }
        blk->add_column(ycol);

        add_block(blk);
    }
}

void BrukerRawDataSet::load_version2(std::istream &f)
{
    meta["format version"] = "2";

    unsigned range_cnt = read_uint16_le(f);

    // add file-scope meta-info
    f.ignore(162);
    meta["DATE_TIME_MEASURE"] = read_string(f, 20);
    meta["CEMICAL SYMBOL FOR TUBE ANODE"] = read_string(f, 2);
    meta["LAMDA1"] = S(read_flt_le(f));
    meta["LAMDA2"] = S(read_flt_le(f));
    meta["INTENSITY_RATIO"] = S(read_flt_le(f));
    f.ignore(8);
    meta["TOTAL_SAMPLE_RUNTIME_IN_SEC"] = S(read_flt_le(f));

    f.ignore(42);   // move ptr to the start of 1st block
    for (unsigned cur_range = 0; cur_range < range_cnt; ++cur_range) {
        Block* blk = new Block;

        // add the block-scope meta-info
        unsigned cur_header_len = read_uint16_le(f);
        format_assert(this, cur_header_len > 48);

        unsigned cur_range_steps = read_uint16_le(f);
        f.ignore(4);
        blk->meta["SEC_PER_STEP"] = S(read_flt_le(f));

        float x_step = read_flt_le(f);
        float x_start = read_flt_le(f);
        StepColumn *xcol = new StepColumn(x_start, x_step);
        blk->add_column(xcol);

        f.ignore(26);
        blk->meta["TEMP_IN_K"] = Su(read_uint16_le(f));

        f.ignore(cur_header_len - 48);  // move ptr to the data_start
        VecColumn *ycol = new VecColumn;
        for(unsigned i = 0; i < cur_range_steps; ++i) {
            float y = read_flt_le(f);
            ycol->add_val(y);
        }
        blk->add_column(ycol);

        add_block(blk);
    }
}

// Contributed by Andreas Breslau.
// Changed by Marcin Wojdyr, based on the file structure specification:
//  DIFFRAC^plus XCH V.5.0 Release 2002. USER'S MANUAL. APPENDIX A.
void BrukerRawDataSet::load_version1_01(std::istream &f)
{
    meta["format version"] = "3";

    // file header - 712 bytes
    // the offset is already 4
    f.ignore(4); // ignore bytes 4-7
    int file_status = read_uint32_le(f);        // address 8
    if (file_status == 1)
        meta["file status"] = "done";
    else if (file_status == 2)
        meta["file status"] = "active";
    else if (file_status == 3)
        meta["file status"] = "aborted";
    else if (file_status == 4)
        meta["file status"] = "interrupted";
    int range_cnt = read_uint32_le(f);          // address 12

    meta["MEASURE_DATE"] = read_string(f, 10);  // address 16
    meta["MEASURE_TIME"] = read_string(f, 10);  // address 26
    meta["USER"] = read_string(f, 72);          // address 36
    meta["SITE"] = read_string(f, 218);         // address 108
    meta["SAMPLE_ID"] = read_string(f, 60);     // address 326
    meta["COMMENT"] = read_string(f,160);       // address 386
    f.ignore(2); // apparently there is a bug in docs, 386+160 != 548
    f.ignore(4); // goniometer code             // address 548
    f.ignore(4); // goniometer stage code       // address 552
    f.ignore(4); // sample loader code          // address 556
    f.ignore(4); // goniometer controller code  // address 560
    f.ignore(4); // (R4) goniometer radius      // address 564
    f.ignore(4); // (R4) fixed divergence...    // address 568
    f.ignore(4); // (R4) fixed sample slit...   // address 572
    f.ignore(4); // primary Soller slit         // address 576
    f.ignore(4); // primary monochromator       // address 580
    f.ignore(4); // (R4) fixed antiscatter...   // address 584
    f.ignore(4); // (R4) fixed detector slit... // address 588
    f.ignore(4); // secondary Soller slit       // address 592
    f.ignore(4); // fixed thin film attachment  // address 596
    f.ignore(4); // beta filter                 // address 600
    f.ignore(4); // secondary monochromator     // address 604
    meta["ANODE_MATERIAL"] = read_string(f,4);  // address 608
    f.ignore(4); // unused                      // address 612
    meta["ALPHA_AVERAGE"] = S(read_dbl_le(f));  // address 616
    meta["ALPHA1"] = S(read_dbl_le(f));         // address 624
    meta["ALPHA2"] = S(read_dbl_le(f));         // address 632
    meta["BETA"] = S(read_dbl_le(f));           // address 640
    meta["ALPHA_RATIO"] = S(read_dbl_le(f));    // address 648
    f.ignore(4); // (C4) unit name              // address 656
    f.ignore(4); // (R4) intensity beta:a1      // address 660
    meta["measurement time"] = S(read_flt_le(f)); // address 664
    f.ignore(43); // unused                     // address 668
    f.ignore(1); // hardware dependency ...     // address 711
    //assert(f.tellg() == 712);

    // range header
    for (int cur_range = 0; cur_range < range_cnt; ++cur_range) {
        Block* blk = new Block;
        int header_len = read_uint32_le(f);     // address 0
        format_assert(this, header_len == 304);
        int steps = read_uint32_le(f);          // address 4
        blk->meta["STEPS"] = S(steps);
        double start_theta = read_dbl_le(f);    // address 8
        blk->meta["START_THETA"]= S(start_theta);
        double start_2theta = read_dbl_le(f);   // address 16
        blk->meta["START_2THETA"] = S(start_2theta);

        f.ignore(8); // Chi drive start         // address 24
        f.ignore(8); // Phi drive start         // address 32
        f.ignore(8); // x drive start           // address 40
        f.ignore(8); // y drive start           // address 48
        f.ignore(8); // z drive start           // address 56
        f.ignore(8);                            // address 64
        f.ignore(6);                            // address 72
        f.ignore(2); // unused                  // address 78
        f.ignore(8); // (R8) variable antiscat. // address 80
        f.ignore(6);                            // address 88
        f.ignore(2); // unused                  // address 94
        f.ignore(4); // detector code           // address 96
        blk->meta["HIGH_VOLTAGE"] = S(read_flt_le(f)); // address 100
        blk->meta["AMPLIFIER_GAIN"] = S(read_flt_le(f)); // 104
        blk->meta["DISCRIMINATOR_1_LOWER_LEVEL"] = S(read_flt_le(f)); // 108
        f.ignore(4);                            // address 112
        f.ignore(4);                            // address 116
        f.ignore(8);                            // address 120
        f.ignore(4);                            // address 128
        f.ignore(4);                            // address 132
        f.ignore(5);                            // address 136
        f.ignore(3); // unused                  // address 141
        f.ignore(8);                            // address 144
        f.ignore(8);                            // address 152
        f.ignore(8);                            // address 160
        f.ignore(4);                            // address 168
        f.ignore(4); // unused                  // address 172
        double step_size = read_dbl_le(f);      // address 176
        blk->meta["STEP_SIZE"] = S(step_size);
        f.ignore(8);                            // address 184
        blk->meta["TIME_PER_STEP"] = S(read_flt_le(f)); // 192
        f.ignore(4);                            // address 196
        f.ignore(4);                            // address 200
        f.ignore(4);                            // address 204
        blk->meta["ROTATION_SPEED [rpm]"] = S(read_flt_le(f));  // 208
        f.ignore(4);                            // address 212
        f.ignore(4);                            // address 216
        f.ignore(4);                            // address 220
        blk->meta["GENERATOR_VOLTAGE"] = Su(read_uint32_le(f)); // 224
        blk->meta["GENERATOR_CURRENT"] = Su(read_uint32_le(f)); // 228
        f.ignore(4);                            // address 232
        f.ignore(4); // unused                  // address 236
        blk->meta["USED_LAMBDA"] = S(read_dbl_le(f)); // 240
        f.ignore(4);                            // address 248
        f.ignore(4);                            // address 252
        int supplementary_headers_size = read_uint32_le(f); // address 256
        f.ignore(4);                            // address 260
        f.ignore(4);                            // address 264
        f.ignore(4);  // unused                 // address 268
        f.ignore(8);                            // address 272
        f.ignore(24); // unused                 // address 280
        //assert(f.tellg() == 712 + (cur_range + 1) * header_len);

        if (supplementary_headers_size > 0)
            f.ignore(supplementary_headers_size);

        StepColumn *xcol = new StepColumn(start_2theta, step_size);
        blk->add_column(xcol);

        VecColumn *ycol = new VecColumn;
        for (int i = 0; i < steps; ++i) {
            float y = read_flt_le(f);
            ycol->add_val(y);
        }
        blk->add_column(ycol);

        add_block(blk);
    }
}

void BrukerRawDataSet::load_version4(std::istream &f)
{
    meta["format version"] = "4";

    // format v4 consists of a 61 byte header (7+54 bytes)
    // followed by a set of metadata segments, followed by
    // one or more ranges corresponding to individual
    // measurement traces.
    // the offset is already 4
    f.ignore(8); // ignore bytes 4-11
    meta["MEASURE_DATE"] = read_string(f, 12);  // address 12
    meta["MEASURE_TIME"] = read_string(f, 10);  // address 24
    f.ignore(27);                               // address 34

    // begin global metadata segment reading
    // here we pretend to undestand only types 10 (0A), 30 (1E), 60 (3C)
    int segment_type = -1;
    int segment_len = 0;
    int drive_num = 0;
    while (1 == 1) {
        segment_type = read_uint32_le(f);       // offset +0
        if (segment_type == 0 || segment_type == 160)
            break; // start of ranges
        segment_len = read_uint32_le(f);        // offset +4
        assert(segment_len >= 8);
        if (segment_type == 10) { // VarInfo
            assert(segment_len >= 36);
            f.ignore(4);                        // offset +8
            string tag_name = read_string(f, 24);      // offset +12
            meta[tag_name] = read_string(f, segment_len-36); // offset +36
        }
        else if (segment_type == 30) { // HardwareConfiguration
            assert(segment_len >= 120);
            f.ignore(64);                               // offset +8
            meta["ALPHA_AVERAGE"] = S(read_dbl_le(f));  // offset +72
            meta["ALPHA1"] = S(read_dbl_le(f));         // offset +80
            meta["ALPHA2"] = S(read_dbl_le(f));         // offset +88
            meta["BETA"] = S(read_dbl_le(f));           // offset +96
            meta["ALPHA_RATIO"] = S(read_dbl_le(f));    // offset +104
            f.ignore(4);                                // offset +112
            meta["ANODE_MATERIAL"] = read_string(f,4);  // offset +116
            f.ignore(segment_len-120);                  // offset +120
        }
        else if (segment_type == 60) { // DriveAlignment
            assert(segment_len >= 76);
            int align_flag = read_uint32_le(f);         // offset +8
            string drive_name = read_string(f, 24);     // offset +12
            f.ignore(32);                               // offset +36
            double delta = read_dbl_le(f);              // offset +68
            f.ignore(segment_len-76);
            meta["DRIVE" + S(drive_num) + "_ALIGN_FLAG"] = S(align_flag);
            meta["DRIVE" + S(drive_num) + "_DELTA"] = S(delta);
            drive_num++;
        }
        else { // skip others
            f.ignore(segment_len-8);
        }
    }

    // now process ranges
    while (segment_type == 0 || segment_type == 160) {
        Block* blk = new Block;
        // primary range header
        f.ignore(28);                               // offset +4
        blk->meta["SCAN_TYPE"] = read_string(f,24); // offset +32
        f.ignore(16);                               // offset +56
        double start_angle = read_dbl_le(f);        // offset +72
        blk->meta["START_ANGLE"] = S(start_angle);
        double step_size = read_dbl_le(f);          // offset +80
        blk->meta["STEP_SIZE"] = S(step_size);
        int steps = read_uint32_le(f);              // offset +88
        blk->meta["STEPS"] = S(steps);
        blk->meta["TIME_PER_STEP"] = S(read_flt_le(f)); // offset +92
        f.ignore(4);                                // offset +96
        blk->meta["GENERATOR_VOLTAGE"] = S(read_flt_le(f)); // +100
        blk->meta["GENERATOR_CURRENT"] = S(read_flt_le(f)); // +104
        f.ignore(4);                                // offset +108
        blk->meta["USED_LAMBDA"] = S(read_dbl_le(f));     // offset +112
        f.ignore(16);                               // offset +120
        int datum_size = read_uint32_le(f);         // offset +136
        int hdr_size = read_uint32_le(f);           // offset +140
        f.ignore(16);                               // offset +144

        // We only grock Locked Coupled and Unlocked Coupled for now
        if (blk->meta["SCAN_TYPE"] == "Locked Coupled" ||
            blk->meta["SCAN_TYPE"] == "Unlocked Coupled") {
            // process ranges for the remaining block headers,
            // ignoring types we don't understand
            while (hdr_size > 0) {
                segment_type = read_uint32_le(f);   // offset +0
                segment_len = read_uint32_le(f);    // offset +4
                assert(segment_len >= 8);
                if (segment_type == 50) {
                    assert(segment_len >= 64);
                    f.ignore(4);                    // offset +8
                    string segment_name = read_string(f,24); // offset +12
                    if (segment_name == "Theta") {
                        f.ignore(20);               // offset +36
                        blk->meta["START_THETA"] = S(read_dbl_le(f)); // +56
                        f.ignore(segment_len-64);
                    }
                    else if (segment_name == "2Theta") {
                        f.ignore(20);               // offset +36
                        blk->meta["START_2THETA"] = S(read_dbl_le(f)); // +56
                        f.ignore(segment_len-64);
                    }
                    else if (segment_name == "Chi") {
                        f.ignore(20);               // offset +36
                        blk->meta["START_CHI"] = S(read_dbl_le(f)); // +56
                        f.ignore(segment_len-64);
                    }
                    else if (segment_name == "Phi") {
                        f.ignore(20);               // offset +36
                        blk->meta["START_PHI"] = S(read_dbl_le(f)); // +56
                        f.ignore(segment_len-64);
                    }
                    else if (segment_name == "BeamTranslation") {
                        f.ignore(20);               // offset +36
                        blk->meta["START_BEAM_TRANSLATION"] = S(read_dbl_le(f)); // +56
                        f.ignore(segment_len-64);
                    }
                    else if (segment_name == "Z-Drive") {
                        f.ignore(20);               // offset +36
                        blk->meta["START_Z-DRIVE"] = S(read_dbl_le(f)); // +56
                        f.ignore(segment_len-64);
                    }
                    else if (segment_name == "Divergence Slit") {
                        f.ignore(20);               // offset +36
                        blk->meta["DIVERGENCE_SLIT"] = S(read_dbl_le(f)); // +56
                        f.ignore(segment_len-64);
                    }
                    else { // ignore others
                        f.ignore(segment_len-36);
                    }
                }
                // Segment type 300 = HRXRD is needed to properly interpret certain
                // scan types. Not implemented here.
                else {
                    f.ignore(segment_len-8);
                }
                hdr_size -= segment_len;
            }

            // Now compute the x values and read the y values
            StepColumn *xcol = new StepColumn(start_angle, step_size);
            blk->add_column(xcol);

            VecColumn *ycol = new VecColumn;
            assert(datum_size == 4);
            for (int i = 0; i < steps; ++i) {
                float y = read_flt_le(f);
                ycol->add_val(y);
            }
            blk->add_column(ycol);
        }
        else { // Skip ranges we don't understand
            blk->meta["UNKNOWN_RANGE_SCAN_TYPE"] = "true";
            f.ignore(hdr_size);
            f.ignore(datum_size*steps);
        }

        add_block(blk);

        // End or next range
        (void)f.peek(); // force eof if at end
        if (f.eof())
            segment_type = -1;
        else
            segment_type = read_uint32_le(f);
    }
}



} // end of namespace xylib

