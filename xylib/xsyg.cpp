#define BUILDING_XYLIB
#include "util.h"
#include "xsyg.h"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/algorithm/string.hpp>
#include <string>
#include <vector>
#include <sstream>

using boost::property_tree::ptree;
typedef ptree::const_assoc_iterator ptiter;

using namespace std;
using namespace xylib::util;

namespace xylib {

const FormatInfo XsygDataSet::fmt_info(
    "xsyg",
    "Freiberg Instruments XSYG",
    "xsyg",
    false,                     // whether binary
    true,                      // whether has multi-blocks
    &XsygDataSet::ctor,
    &XsygDataSet::check
);

bool XsygDataSet::check(std::istream &f, string*)
{
    ptree tree;
    read_xml(f, tree);
    //check if child "Sample" is part of the XML file
    return tree.count("Sample") != 0;
}

void XsygDataSet::load_data(std::istream &f, const char*) {
    ptree tree;
    read_xml(f, tree);
    ptree sample = tree.get_child("Sample");

    //store metaData
    meta["state"] = sample.get("<xmlattr>.state", "");
    meta["name"] = sample.get("<xmlattr>.name", "");
    meta["user"] = sample.get("<xmlattr>.user", "");
    meta["startDate"] = sample.get("<xmlattr>.startDate", "");
    meta["sampleCarrier"] = sample.get("<xmlattr>.sampleCarrier", "");
    meta["lexsygID"] = sample.get("<xmlattr>.lexsygID", "");
    meta["lexStudioVersion"] = sample.get("<xmlattr>.lexStudioVersion", "");
    meta["firmwareVersion"] = sample.get("<xmlattr>.firmwareVersion", "");
    meta["os"] = sample.get("<xmlattr>.os", "");
    meta["comment"] = sample.get("<xmlattr>.comment", "");

    int AQ_nr = 0;
    std::pair<ptiter, ptiter> sequenceRange = sample.equal_range("Sequence");
    for (ptiter it_seq = sequenceRange.first; it_seq != sequenceRange.second; ++it_seq) {
        ++AQ_nr;
        std::pair<ptiter, ptiter> recordRange = it_seq->second.equal_range("Record");
        int measurement_nr = 0;
        //loop Measurements
        for (ptiter i = recordRange.first; i != recordRange.second; ++i) {
            std::pair<ptiter, ptiter> curveRange = i->second.equal_range("Curve");
            string recordType = i->second.get("<xmlattr>.recordType", "");
            //loop Curves
            for (ptiter j = curveRange.first; j != curveRange.second; ++j) {
                if (j->second.get("<xmlattr>.curveType", "") != "measured")
                    continue;
                if (j->second.get("<xmlattr>.detector", "") == "")
                    continue;
                Block *blk = new Block;
                ++measurement_nr;
                string curveDesc = j->second.get("<xmlattr>.curveDescripter", "");
                std::string x_desc, y_desc;
                str_split(curveDesc, ';', x_desc, y_desc);
                blk->meta["curveDescriptor"] = curveDesc;
                blk->meta["state"] = j->second.get("<xmlattr>.state", "");
                string detector = j->second.get("<xmlattr>.detector", "");
                blk->meta["detector"] = detector;
                blk->meta["startDate"] = j->second.get("<xmlattr>.startDate", "");
                blk->meta["offset"] = j->second.get("<xmlattr>.offset", "");
                VecColumn *x_col = new VecColumn;
                x_col->set_name(x_desc);
                std::istringstream curve_ss(j->second.data());
                if (j->second.get("<xmlattr>.detector", "") != "Spectrometer") {
                    VecColumn *y_col = new VecColumn;
                    y_col->set_name(y_desc);

                    // read data between <Curve> ... </Curve>
                    std::string token;
                    while (std::getline(curve_ss, token, ';')) {
                        size_t pos = token.find(',');
                        if (pos != std::string::npos) {
                            x_col->add_val(my_strtod(token.substr(0, pos)));
                            y_col->add_val(my_strtod(token.substr(pos+1)));
                        }
                    }

                    blk->meta["stimulator"] = j->second.get("<xmlattr>.stimulator", "");

                    blk->add_column(x_col);
                    blk->add_column(y_col);
                } else { // detector="Spectrometer"
                    // read wavelength from attribute "wavelengthTable"
                    std::string wavelengths = j->second.get("<xmlattr>.wavelengthTable", "");
                    x_col->add_values_from_str(wavelengths, ';');
                    blk->add_column(x_col);

                    // read data between <Curve> ... </Curve>
                    std::string token;
                    while (std::getline(curve_ss, token, ';')) {
                        size_t pos = token.find(',');
                        size_t open_br = token.find('[', pos);
                        size_t close_br = token.find(']', open_br);
                        if (close_br != std::string::npos) {
                            VecColumn *y_col = new VecColumn;
                            y_col->set_name(str_trim(token.substr(0, pos)));
                            // read [y1|y2|...yn]
                            std::string ystr(token, open_br+1, close_br-open_br-1);
                            y_col->add_values_from_str(ystr, '|');
                            blk->add_column(y_col);
                        }
                    }

                    //add meta data
                    blk->meta["startDate"] = j->second.get("<xmlattr>.startDate", "");
                    blk->meta["duration"] = j->second.get("<xmlattr>.duration", "");
                    blk->meta["calibration"] = j->second.get("<xmlattr>.calibration", "");
                    blk->meta["cameraType"] = j->second.get("<xmlattr>.cameraType", "");
                    blk->meta["integrationTime"] = j->second.get("<xmlattr>.integrationTime", "");
                    blk->meta["channelTime"] = j->second.get("<xmlattr>.channelTime", "");
                    blk->meta["CCD_temperature"] = j->second.get("<xmlattr>.CCD_temperature", "");
                }
                ostringstream name;
                name << "AQ: " << AQ_nr << ", Meas.: " << measurement_nr
                     << ", Type: "  << recordType << " ("  << detector  << ')';
                blk->set_name(name.str());
                add_block(blk);
            } // end loop curves
        } // end loop measurements
    }
} // end load_data

}// namespace xylib
