// Ron Unwin's Spectra Omicron XPS data file
// Licence: Lesser GNU Public License 2.1 (LGPL)
// Author: Matthias Richter

// Format used by old DOS program called SPECTRA, written by R. Unwin.
// In the manual R. Unwin describes the file format.
//
// First page of the manual:
//                                SPECTRA
//                               VERSION 8
//                       Graphical User Interface
//                 Programs for Spectroscopy & Imaging
//     Copyright (c) R Unwin 1989 to 2001.
//     Created using Borland Pascal, Borland Pascal, Borland Delphi and
//     Borland C++ Builder ...

// From email from M. Richter:
//   Included in the software suite is also a program to
//   load the data called PRESENTS which can handle various data
//   formats.  Here two formats are of interest: "VGX 900 data" and
//   "SPECTRA data" (these are the official names R.  Unwin uses).
//   R. Unwin writes that both are fully compatible but we should name this
//   format "SPECTRA data" or "SPECTRA format" anyway, because this is
//   the one which the program "SPECTRA" uses.  And the official file
//   encoding is ASCII.

#ifndef XYLIB_SPECTRA_H_
#define XYLIB_SPECTRA_H_
#include "xylib.h"

namespace xylib {

    class SpectraDataSet : public DataSet
    {
        OBLIGATORY_DATASET_MEMBERS(SpectraDataSet)
    private:
        Block* read_block(std::istream& f);
    };

}
#endif // XYLIB_SPECTRA_H_

