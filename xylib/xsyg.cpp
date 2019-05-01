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
    "Freiberg Instruments (FI) Lexsyg",
    "xsyg",
    false,                      // whether binary
    true,                      // whether has multi-blocks
    &XsygDataSet::ctor,
    &XsygDataSet::check
);

bool XsygDataSet::check(std::istream &f, string*)
{

    ptree tree;
    read_xml(f, tree);

    //check if child "Sample" is part of the XML file
    if(tree.count("Sample") == 0 ){
        return false;
    } else {
        return true;
    }

}

void XsygDataSet::load_data(std::istream &f, const char*) {

    ptree tree;
    unsigned int measurement_nr, AQ_nr = 1;

    //read XML file
    read_xml(f, tree);

    ptree sample = tree.get_child("Sample");

    //store metaData

    std::string state, parentID, name, user, startDate, sampleCarrier, lexsygID, lexStudioVersion, firmwareVersion,
    os, comment;

    state = sample.get("<xmlattr>.state","");
    parentID = sample.get("<xmlattr>.parentID","");
    name = sample.get("<xmlattr>.name","");
    user = sample.get("<xmlattr>.user","");
    startDate = sample.get("<xmlattr>.startDate","");
    sampleCarrier = sample.get("<xmlattr>.sampleCarrier","");
    lexsygID = sample.get("<xmlattr>.lexsygID","");
    lexStudioVersion = sample.get("<xmlattr>.lexStudioVersion","");
    firmwareVersion = sample.get("<xmlattr>.firmwareVersion","");
    os = sample.get("<xmlattr>.os","");
    comment = sample.get("<xmlattr>.comment","");

    meta["state"] = state;
    meta["parentID"] = parentID;
    meta["name"] = name;
    meta["user"] = user;
    meta["StartDate"] = startDate;
    meta["sampleCarrier"] = sampleCarrier;
    meta["lexsygID"] = lexsygID;
    meta["lexStuidoVersion"] = lexStudioVersion;
    meta["firmwareVersion"] = firmwareVersion;
    meta["os"] = os;
    meta["comment"] = comment;

    //loop Aliquots
    std::pair<ptiter, ptiter> sequenceRange = sample.equal_range("Sequence");

    for (ptiter it_seq = sequenceRange.first; it_seq != sequenceRange.second; ++it_seq, ++AQ_nr) {

    std::pair<ptiter, ptiter> recordRange = it_seq->second.equal_range("Record");

    //set counter = 0
    measurement_nr = 0;

    //loop Measurements
    for (ptiter i = recordRange.first; i != recordRange.second; ++i) {

        std::pair<ptiter, ptiter> curveRange = i->second.equal_range("Curve");

        //loop Curves
        for (ptiter j = curveRange.first; j != curveRange.second; ++j) {

            if(j -> second.get("<xmlattr>.curveType","") == "measured"){

                if(j -> second.get("<xmlattr>.detector","") != ""){

                    if(j -> second.get("<xmlattr>.detector","") != "Spectrometer"){

                    ++measurement_nr;

                    //create new Block, x- and y-axis
                    Block *blk = new Block;
                    VecColumn *x_axis_col = new VecColumn;
                    VecColumn *y_axis_col = new VecColumn;

                    //read data between <Curve> ... </Curve>
                    std::string data = j->second.data();

                    std::istringstream ss(data);
                    std::string token, token2;

                    //set counter 0 for decision if x- or y-axis
                    int it_data = 0;

                    while(std::getline(ss, token, ';')) {

                        std::istringstream ss2(token);

                        while(std::getline(ss2, token2, ',')) {

                            it_data++;

                            //take every second element as y-value
                            if(it_data %2 == 1){
                                x_axis_col-> add_values_from_str(token2);
                            } else {
                                y_axis_col-> add_values_from_str(token2);
                            }

                        }// end 2nd while-loop: token 2
                    }// end 1st while-loop: token


                    //extract meta data
                    string curveDescriptor = j -> second.get("<xmlattr>.curveDescripter","");
                    std::vector<std::string> curve_descriptor_split;
                    boost::split(curve_descriptor_split, curveDescriptor, boost::is_any_of(";"));

                    string recordType = i -> second.get("<xmlattr>.recordType","");
                    string detector = j -> second.get("<xmlattr>.detector","");
                    string state = j -> second.get("<xmlattr>.state","");
                    string parentID = j-> second.get("<xmlattr>.state","");
                    string startDate = j -> second.get("<xmlattr>.startDate","");
                    string curveType = j -> second.get("<xmlattr>.curveType","");
                    string stimulator = j -> second.get("<xmlattr>.simulator","");
                    string offset= j -> second.get("<xmlattr>.offset","");

                    //add meta data
                    blk->meta["state"] = state;
                    blk->meta["parentID"] = parentID;
                    blk->meta["detector"] = detector;
                    blk->meta["startDate"] = startDate;
                    blk->meta["curveType"] = curveType;
                    blk->meta["stimulator"] = stimulator;
                    blk->meta["curveDescriptor"] = curveDescriptor;
                    blk->meta["offset"] = offset;


                    //set names of columns
                    x_axis_col-> set_name(curve_descriptor_split[0]);
                    y_axis_col-> set_name(curve_descriptor_split[1]);

                    //set name of block
                    ostringstream convert_AQ, convert_measurement_nr;
                    convert_AQ << AQ_nr;
                    convert_measurement_nr << measurement_nr;

                    blk->set_name("AQ: " + convert_AQ.str() + ", Meas.: " +
                            convert_measurement_nr.str() + ", Type: " + recordType + " (" + detector +")");

                    // add column to block
                    blk->add_column(x_axis_col);
                    blk->add_column(y_axis_col);

                    //finally: add block
                    add_block(blk);

                    } else {	// end if not spectrometer

                    ++measurement_nr;

                    //create new Block, x- and y-axis
                    Block *blk = new Block;
                    VecColumn *x_axis_col = new VecColumn;

                    //read wavelength from attribute "wavelengthTable"
                    std::string wavelength = j->second.get("<xmlattr>.wavelengthTable","");
                    std::istringstream wavelength_ss(wavelength);
                    std::string wavelength_split;

                    while(std::getline(wavelength_ss, wavelength_split, ';')) {
                        x_axis_col-> add_values_from_str(wavelength_split);
                    }

                    blk->add_column(x_axis_col);

                    //read counts
                    std::string intens = j->second.data();
                    std::istringstream data_ss(intens);
                    std::string data_split, data_split_2, data_split_3;

	            //set counter 0 for decision if x- or y-axis
	            int it_data = 0;

                    while(std::getline(data_ss, data_split, ';')) {

	                std::istringstream data_ss2(data_split);

	                while(std::getline(data_ss2, data_split_2, ',')) {

	                	VecColumn *y_col_intens = new VecColumn;

                            it_data++;
	                    if(it_data %2 == 1){
//	                        times.push_back(data_split_2);
	                    }else {

                                // remove [ and ] from string
	                        int last_br = data_split_2.find("]");
	                        data_split_2.erase(last_br,1);
	                    	data_split_2.erase(0,1);

	                    	std::istringstream data_ss3(data_split_2);

	                    	while(std::getline(data_ss3, data_split_3, '|')) {

	                            y_col_intens -> add_values_from_str(data_split_3);

	                    	}

	                    	y_col_intens-> set_name("cts [1/ch]");

	                    	blk->add_column(y_col_intens);
	                    } // end else
                        } // end while 2
                    } // end while 1

                    //get curveDescripter
                    string curveDescriptor = j -> second.get("<xmlattr>.curveDescripter","");
                    std::vector<std::string> curve_descriptor_split;
                    boost::split(curve_descriptor_split, curveDescriptor, boost::is_any_of(";"));

                    //set names of columns
                    x_axis_col-> set_name(curve_descriptor_split[1]);

                    string recordType = i -> second.get("<xmlattr>.recordType","");
                    string state = j -> second.get("<xmlattr>.state","");
                    string parentID = j-> second.get("<xmlattr>.state","");
                    string startDate = j -> second.get("<xmlattr>.startDate","");
                    string curveType = j -> second.get("<xmlattr>.curveType","");
                    string detector = j -> second.get("<xmlattr>.detector","");
                    string offset= j -> second.get("<xmlattr>.offset","");
                    string duration = j -> second.get("<xmlattr>.duration","");
                    string calibration = j -> second.get("<xmlattr>.calibration","");
                    string cameraType = j -> second.get("<xmlattr>.cameraType","");
                    string integrationTime = j -> second.get("<xmlattr>.integrationTime","");
                    string channelTime = j -> second.get("<xmlattr>.channelTime","");
                    string CCD_temperature = j -> second.get("<xmlattr>.CCD_temperature","");

                    //add meta data
                    blk->meta["state"] = state;
                    blk->meta["parentID"] = parentID;
                    blk->meta["startDate"] = startDate;
                    blk->meta["curveTyp"] = curveType;
                    blk->meta["detector"] = detector;
                    blk->meta["offset"] = offset;
                    blk->meta["duration"] = duration;
                    blk->meta["calibration"] = calibration;
                    blk->meta["cameraType"] = cameraType;
                    blk->meta["integrationTime"] = integrationTime;
                    blk->meta["channelTime"] = channelTime;
                    blk->meta["CCD_temperature"] = CCD_temperature;
                    blk->meta["curveDescriptor"] = curveDescriptor;

                    //set name of block
                    ostringstream convert_AQ, convert_measurement_nr;
                    convert_AQ << AQ_nr;
                    convert_measurement_nr << measurement_nr;

                    blk->set_name("AQ: " + convert_AQ.str() + ", Meas.: " +
                            convert_measurement_nr.str() + ", Type: " + recordType + " (" + detector +")");

                    //add meta data
                    blk->meta["detector"] = detector;
                    blk->meta["state"] = state;
                    blk->meta["parentID"] = parentID;
                    blk->meta["startDate"] = startDate;

                    //finally: add block
                    add_block(blk);

                    } // end if Spectrometer
                } // end if detector != ""
            } // end if measured
        } // end loop curves
    } // end loop measurements
    } // end loop aliquots


} // end load_data

}// namespace xylib
