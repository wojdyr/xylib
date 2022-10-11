// PANalytical XRDML
// Licence: Lesser GNU Public License 2.1 (LGPL)

#define BUILDING_XYLIB
#include "xrdml.h"

#include <cstring>
#include <memory>  // for unique_ptr
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include "util.h"

using namespace std;
using namespace xylib::util;
using boost::property_tree::ptree;
typedef ptree::const_assoc_iterator ptiter;

namespace xylib {

const FormatInfo XrdmlDataSet::fmt_info(
    "xrdml",
    "PANalytical XRDML",
    "xrdml",
    false,                     // whether binary
    true,                      // whether has multi-blocks
    &XrdmlDataSet::ctor,
    &XrdmlDataSet::check
);

// Check for "www.xrdml.com" which is part of xmlns and in xsi:schemaLocation.
// Normally it is at the beginning of the file.
bool XrdmlDataSet::check(istream &f, string*)
{
    char buf[1024];
    memset(buf, 0, sizeof(buf));
    f.read(buf, sizeof(buf) - 1);
    return strstr(buf, "www.xrdml.com") != NULL;
}

static Block* make_block_from_points(const ptree& data_points)
{
    std::unique_ptr<Block> blk(new Block);
    StepColumn *xs_col = NULL;
    std::pair<ptiter, ptiter> prange = data_points.equal_range("positions");
    for (ptiter i = prange.first; i != prange.second; ++i) {
        const ptree& pos = i->second;
        string x_axis = pos.get<string>("<xmlattr>.axis");
        if (pos.count("startPosition") != 0) {
            double x_start = my_strtod(pos.get<string>("startPosition"));
            double x_end = my_strtod(pos.get<string>("endPosition"));
            // the step will be later divided by number-of-points - 1
            xs_col = new StepColumn(x_start, x_end - x_start);
            blk->add_column(xs_col);
            xs_col->set_name(x_axis);
            break;
        } else if (pos.count("listPositions") != 0) { // untested - no examples
            string str = pos.get<string>("listPositions");
            VecColumn *xv_col = new VecColumn;
            blk->add_column(xv_col);
            xv_col->add_values_from_str(str);
            xv_col->set_name(x_axis);
            break;
        }
    }
    if (blk->get_column_count() == 0)
        throw FormatError("cannot deduce x values");

    string inten_str = data_points.get<string>("counts");
    VecColumn *ycol = new VecColumn;
    blk->add_column(ycol);
    ycol->add_values_from_str(inten_str);
    if (ycol->get_point_count() < 2)
        string inten_str = data_points.get<string>("intensities");
        VecColumn *ycol = new VecColumn;
        blk->add_column(ycol);
        ycol->add_values_from_str(inten_str);
        if (ycol->get_point_count() < 2)
            throw FormatError("intensities/counts do not look correct");
    if (xs_col != NULL)
        xs_col->set_step(xs_col->get_step() / (ycol->get_point_count() - 1));
    //blk->set_name(title);
    return blk.release();
}

void XrdmlDataSet::load_data(std::istream &f, const char*)
{
    ptree tree;
    try {
        boost::property_tree::read_xml(f, tree);
        ptree top = tree.get_child("xrdMeasurements");
        std::pair<ptiter, ptiter> mrange = top.equal_range("xrdMeasurement");
        // todo: xrdMeasurement/usedWavelength -> metadata
        for (ptiter i = mrange.first; i != mrange.second; ++i) {
            std::pair<ptiter, ptiter> srange = i->second.equal_range("scan");
            for (ptiter j = srange.first; j != srange.second; ++j) {
                ptree data_points = j->second.get_child("dataPoints");
                add_block(make_block_from_points(data_points));
            }
        }
    } catch (boost::property_tree::ptree_error& e) {
        throw FormatError(e.what());
    }
}

} // namespace xylib

