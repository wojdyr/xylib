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

void XsygDataSet::load_data(std::istream &f){

	ptree tree;
	unsigned int measurement_nr, AQ_nr = 1;

	//read XML file
	read_xml(f, tree);

	ptree sample = tree.get_child("Sample");

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
								double db_tok2_x = strtod(token2.c_str(), NULL);
								x_axis_col-> add_val(db_tok2_x);

							} else {
								double db_tok2_y = strtod(token2.c_str(), NULL);
								y_axis_col-> add_val(db_tok2_y);
							}

	            		}// end 2nd while-loop: token 2
	            	}// end 1st while-loop: token


	            	//get curveDescripter
	            	string curveDescriptor = j -> second.get("<xmlattr>.curveDescripter","");
	            	std::vector<std::string> curve_descriptor_split;
	            	boost::split(curve_descriptor_split, curveDescriptor, boost::is_any_of(";"));

	            	//get recordType
	            	string recordType = i -> second.get("<xmlattr>.recordType","");
	            	string detector = j -> second.get("<xmlattr>.detector","");

	            	//get state
	            	string state = j -> second.get("<xmlattr>.state","");

	            	//get parentID
	            	string parentID = j-> second.get("<xmlattr>.state","");

	            	//get startDate
	            	string startDate = j -> second.get("<xmlattr>.startDate","");

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

		        	//add meta data
		        	blk->meta["detector"] = detector;
		        	blk->meta["state"] = state;
		        	blk->meta["parentID"] = parentID;
		        	blk->meta["startDate"] = startDate;

		        	//finally: add block
		        	add_block(blk);

					} // end if detector != ""
	          } // end if measured


		} // end 2nd for
	   	}

	} // end loop aliquots


} // end load_data

}// namespace xylib
