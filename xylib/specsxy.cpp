// Specslab XY data file
// Licence: Lesser GNU Public License 2.1 (LGPL)
// Author: Matthias Richter

// based on LoadSpecsXYFiles.ipf (IgorPro)
// copyright 2006-2007 by SPECS GmbH , Berlin, Germany
// Import Filter for xy files produced by SpecsLab2
// (specs.de)


#define BUILDING_XYLIB
#define UNICODE
#include "specsxy.h"

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <iomanip>
#include "util.h"
#include <algorithm>
#include <string>

#define  MarkerScanModeFixedTransm  "FixedAnalyzerTransmission"
#define  MarkerScanModeFixedEnerg   "FixedEnergies"
#define  MarkerScanModeCFS          "ConstantFinalState"
#define  MarkerScanModeCIS          "ConstantInitialState"
#define  MarkerScanModeDetector     "DetectorVoltageScan"
#define  ScanMode_FixedTransm       1
#define  ScanMode_FixedEnerg        2
#define  ScanMode_CFS               3
#define  ScanMode_CIS               4
#define  ScanMode_Detector          5


using namespace std;
using namespace xylib::util;
using std::setprecision;
using std::fixed;


namespace {

    void Debugprintf(string /*str*/, string /*out*/){
        //comment in for debugging
        //printf(str.c_str(), out.c_str());
    }

    string doubleToString(double d){
        stringstream ss;
        ss <<setprecision(15)<< d;
        return (ss.str());
    }

    string intToString(int i){
        stringstream ss;
        ss << i;
        return (ss.str());
    }

	struct InvalidChar
	{
	    bool operator()(char c) const {
	        return !isprint((unsigned)c);
	    }
	};


        /*
	string stripstr(string str, char in, char out){
		replace(str.begin(), str.end(), in, out);
		return str;
	}
        */


	string cleanstr(string str){
		//stringPurifier(str); // or next
		str.erase(std::remove_if(str.begin(),str.end(),InvalidChar()), str.end());
		return str;
	}

    /*
    int grepDataTitle(const char * str, int *cycle, int *curve, int *scan, int *channel){
        int numRead=0, tmpcycle=0, tmpcurve=0, tmpscan=0, tmpchannel=0;
        string out="";
        numRead=sscanf(str, "# Cycle: %d, Curve: %d, Scan: %d, Channel: %d", &tmpcycle, &tmpcurve, &tmpscan, &tmpchannel);

        switch(numRead)
        {
            case 4:
                *channel=tmpchannel;
                break;
            case 3:
                *scan=tmpscan;
                break;
            case 2:
                *curve=tmpcurve;
                *cycle=tmpcycle;
                break;
            default:
                break;
        }
        Debugprintf ("grepDataTitle is " + out + " , and num is %s\n", intToString(numRead));
        Debugprintf("V_flag is%s\n", intToString(numRead));
        return numRead;
    }
    */

    string grepLine(string str, string *option){
        size_t posdp=0,posspace=0;
        Debugprintf("line to examine is %s\n", str);

        posdp = str.find_first_of(":");
        posspace = str.find_first_of(" ");
        if( posspace == string::npos)
        {
            *option="";
            return "";
        }

        if( posdp == string::npos)  // needed to support multiline comments
        {
            *option = str.substr(posspace,str.length());
        }else{
            *option = str.substr(posdp+1,str.length());
        }
        *option = str_trim(*option);
        str = str.substr(posspace+1,posdp-1);
        Debugprintf("Type is _%s_\n",str);
        Debugprintf("Option is _%s_\n",*option);
        return str;
    }

    bool isDataLine(const char * str, double *xval, double *yval){
        int valRead=0, ret=1;
        double tmpx=0.0, tmpy=0.0;
        valRead=sscanf(str, "%lg \t%lg", &tmpx, &tmpy);
        if(valRead != 2){
            valRead=sscanf(str, "%lg\t%lg", &tmpx, &tmpy);
            if(valRead != 2){
                ret =  0;
            }
        }
        if (valRead == 1){
            return false;
        }
        if (ret == 0){
            return false;
        }
        *xval = tmpx;
        *yval = tmpy;
        Debugprintf("Found data line containing: %s\n",doubleToString(*xval) + " " + doubleToString(*yval));
        return true;
    }

    bool checkEnergyScale(vector<double> * w,int num, double eps)
    {
        for (int i = 1; i < num; ++i) // check to see if we have aquidistante energy scale
        {
            if( abs( w->at(1) - w->at(0) - ( w->at(i) - w->at(i-1)) )  >= eps)
            {
                return false;
            }
        }
        return true;
    }

}


namespace xylib {


const FormatInfo SpecsxyDataSet::fmt_info(
    "specsxy",
    "SPECS Specslab2 xy",
    "xy",
    false,                     // whether binary
    true,                      // whether has multi-blocks
    &SpecsxyDataSet::ctor,
    &SpecsxyDataSet::check
);

bool SpecsxyDataSet::check(istream &f, string*){
        string TmpS = "";
        getline(f, TmpS);
        if(TmpS.find("# Created by:        SpecsLab2, Version 2.",0) != 0)
        {
            return false;
        }

    return true;
} // SpecsxyDataSet::check

void SpecsxyDataSet::load_data(std::istream &f){
        Block* blk = NULL;
        int line=0, numLinesLoaded=0, cmtLine=-1, n_wave=1;
        //int datacounter=0, cnt=0, cycleHeader=0, num=0;
        string cycleheader = "";
        bool aquedist = false;
        double startx=0.0, stepx=0.0;
        vector<VecColumn*> yw;
        yw.push_back(new VecColumn);
        vector<double> xw;
        xw.resize(0);
        double eps=1E-6; // used in checkEnergyScale(...) to determine if two adjacent energy intervals are equal
        string str="", type ="????", group="", /*spectrum="",*/ fname="", newfname="", oldgroup="", tmp="";
        string method="", lens="", slit="", analyzer="", mode="", curvesPerScan="", valuesPerCurve="",dwellTime="", excEnergy="", passEnergy="", biasVoltage="";
        string detVoltage="", effWorkfunction="", source="", numberOfScan="", kinEnergy="", remote="", comment="", region="", energy="", remoteInfo="";
        int i1=0;
        int cycle=0, scan=0, curve=0, channel=0;
        double xval=0.0, yval=0.0;
        string option="";
		string xcolname = "";
        bool read = true;
		int ScanMode = 0;

        while(read){
            getline(f, str);
            line+=1;
            if (f.eof()){// hit EOF
                    read= false;
                    break;
            }
            str=str_trim(str);
            if(isDataLine(str.c_str(),&xval,&yval)){
                if(numLinesLoaded == 0){ // new block begins
                    /*
                    if(group.compare("") != 0 && oldgroup.compare(group) != 0){ // new group
                        oldgroup = group;
                    }

                    if( region.compare("") == 0 ){// take save default name
                        datacounter+=1;
                        spectrum = "data" + intToString(datacounter);
                    }else{
                        spectrum = region;
                    }
                    if( cycle > 0){
                        spectrum += "Cy" + intToString(cycle);
                    }
                    if( my_strtol(numberOfScan) > 1 && cycleHeader  >= 3 ){ // we have separate data for each scan
                        spectrum += "Sc" + intToString(scan);
                    }
                    if (my_strtol(curvesPerScan) > 1){
                        spectrum += "Cu" + intToString(curve);
                    }
                    if ( cycleHeader == 4){ // separate data for each channel
                        spectrum += "Ch" + intToString(channel);
                    }
                    energy =  "En_" + spectrum;
                    */
                    xw.resize(0);
                    yw.pop_back();
                    yw.push_back(new VecColumn);

                }

                xw.push_back(xval);
                yw[0]->add_val(yval);
                numLinesLoaded+=1;
            }else{


                if(numLinesLoaded != 0){
                    aquedist = checkEnergyScale(&xw,numLinesLoaded,eps);
                    numLinesLoaded=0;
                    if(aquedist == true){ // == 1
                        startx = xw[0];
                        stepx = xw[1] - xw[0];
                    }else{
                        // they should be aquedistant, if not --> error
                        return;
                    }
                    blk = new Block;
                    blk->set_name(region);

	                switch(ScanMode)
	                {
	                    case ScanMode_FixedTransm:
                        	startx -= my_strtod(excEnergy); // negative binding energy for XPS
                        	xcolname = "bindingenergy [eV]";
	                    break;
	                    case ScanMode_FixedEnerg:
                        	startx = 0;
                        	stepx =  my_strtod(dwellTime);
                        	xcolname = "time [s]";
	                    break;
	                    case ScanMode_CFS:
                        	startx = my_strtod(excEnergy);
                        	//stepx = my_strtod();
                        	xcolname = "photon energy [eV]";
	                    break;
	                    case ScanMode_CIS:
                        	startx = my_strtod(excEnergy);
                        	//stepx = my_strtod();
                        	xcolname = "photon energy [eV]";
	                    break;
	                    case ScanMode_Detector:
                        	//startx = my_strtod(detVoltage); // get a sample xy to see whats right
                        	//stepx = my_strtod();
                        	xcolname = "detector voltage [V]";
						break;
						default:
							//nothing yet
						break;
					}

                    StepColumn *xcol = new StepColumn(startx, stepx);
                    xcol->set_name(xcolname);
                    yw[0]->set_name(region+" [cps]");
                    blk->add_column(xcol);
                    blk->add_column(yw[0]);
                    blk->meta["Group"]                      = group;
                    blk->meta["Region"]                     = region;
                    blk->meta["Number of Scans"]            = numberOfScan;
                    blk->meta["Curves/Scan"]                = curvesPerScan;
                    blk->meta["Cycle"]                      = intToString(cycle);
                    blk->meta["Scans"]                      = intToString(scan);
                    blk->meta["Curve"]                      = intToString(curve);
                    blk->meta["Channel"]                    = intToString(channel);
                    blk->meta["Method"]                     = method;
                    blk->meta["Analyzer"]                   = analyzer;
                    blk->meta["Analyzer Lens"]              = lens;
                    blk->meta["Analyzer Slit"]              = slit;
                    blk->meta["Scan Mode"]                  = mode;
                    blk->meta["Dwell time"]                 = dwellTime;
                    blk->meta["Excitation energy"]          = excEnergy;
                    blk->meta["Kinetic energy"]             = kinEnergy;
                    blk->meta["Pass energy"]                = passEnergy;
                    blk->meta["Bias Voltage"]               = biasVoltage;
                    blk->meta["Detector Voltage"]           = detVoltage;
                    blk->meta["Effective Workfunction"]     = effWorkfunction;
                    blk->meta["Source"]                     = source;
                    blk->meta["Remote"]                     = remote;
                    blk->meta["RemoteInfo"]                 = remoteInfo;
                    blk->meta["Comment"]                    = comment;

                    // all data of one block loaded --> add data to blk and than add blk
                    add_block(blk);
                    n_wave  += 1;
                }
                //num = grepDataTitle(str.c_str(), &cycle, &curve, &scan, &channel);
                /*if( num >= 2 ){
                    cycleheader = num;
                    continue;
                }*/

                str = grepLine(str, &option);
                i1=0;
                if (str.compare("Group:") == 0){
                    group=cleanstr(option);
                    i1=1;
                }
                if(str.compare("Region:") == 0){
                    region = cleanstr(option);
                    i1=1;
                }
                if(str.compare("Anylsis Method:") == 0){
                    method = option;
                    i1=1;
                }
                if(str.compare("Analysis Method:") == 0){
                    method = option;
                    i1=1;
                }
                if(str.compare("Analyzer:") == 0){
                    analyzer = option;
                    i1=1;
                }
                if(str.compare("Analyzer Lens:") == 0){
                    lens = option;
                    i1=1;
                }
                if(str.compare("Analyzer Slit:") == 0){
                    slit = option;
                    i1=1;
                }
                if(str.compare("Scan Mode:") == 0){
                    mode = option;
	                if (mode.find(MarkerScanModeFixedTransm,0) == 0)    ScanMode = ScanMode_FixedTransm;
	                if (mode.find(MarkerScanModeFixedEnerg,0)  == 0)    ScanMode = ScanMode_FixedEnerg;
	                if (mode.find(MarkerScanModeCFS,0)         == 0)    ScanMode = ScanMode_CFS;
	                if (mode.find(MarkerScanModeCIS,0)         == 0)    ScanMode = ScanMode_CIS;
	                if (mode.find(MarkerScanModeDetector,0)    == 0)    ScanMode = ScanMode_Detector;
                    i1=1;
                }
                if(str.compare("Number of Scans:") == 0){
                    numberOfScan = option;
                    i1=1;
                }
                if(str.compare("Curves/Scan:") == 0){
                    curvesPerScan = option;
                    i1=1;
                }
                if(str.compare("Values/Curve:") == 0){
                    valuesPerCurve= option;
                    i1=1;
                }
                if(str.compare("Dwell Time:") == 0){
                    dwellTime = option;
                    i1=1;
                }
                if(str.compare("Excitation Energy:") == 0){
                    excEnergy = option;
                    i1=1;
                }
                if(str.compare("Kinetic Energy:") == 0){
                    kinEnergy = option;
                    i1=1;
                }
                if(str.compare("Pass Energy:") == 0){
                    passEnergy = option;
                    i1=1;
                }
                if(str.compare("Bias Voltage:") == 0){
                    biasVoltage = option;
                    i1=1;
                }
                if(str.compare("Detector Voltage:") == 0){
                    detVoltage = option;
                    i1=1;
                }
                if(str.compare("Eff. Workfunction:") == 0){
                    effWorkfunction = option;
                    i1=1;
                }
                if(str.compare("Source:") == 0){
                    source = option;
                    i1=1;
                }
                if(str.compare("RemoteInfo:") == 0){
                    remoteInfo = option;
                    i1=1;
                }
                if(str.compare("Remote:") == 0 ){
                    remote = option;
                    i1=1;
                }
                if(str.compare("Comment:") == 0){
                    cmtLine = line;
                    comment = cleanstr(option);
                    i1=1;
                }
                if(i1==0){
                    if( ( cmtLine + 1 ) == line && option.compare("") != 0 ){
                        cmtLine +=1;
                        //comment  = comment + "\n" + cleanstr(option); // can xylib handle multiple comment lines??
                        comment = comment + " " + cleanstr(option);
                    }else{
                        cmtLine=-1;
                    }
                }
            }//endif
        }//endwhile
} // SpecsxyDataSet::load_data

} // namespace xylib

