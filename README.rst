
=====
xylib
=====

xylib is a portable library for reading files that contain x-y data from
powder diffraction, spectroscopy and other experimental methods.

Supported formats:

-  plain text, delimiter-separated values (e.g. CSV)
-  Crystallographic Information File for Powder Diffraction (pdCIF)
-  Siemens/Bruker UXD
-  Siemens/Bruker RAW ver. 1/2/3
-  Philips UDF
-  Philips RD (raw scan) V3
-  Rigaku DAT
-  Sietronics Sieray CPI
-  DBWS/DMPLOT data file
-  Canberra CNF (from Genie-2000 software; aka CAM format)
-  Canberra AccuSpec MCA
-  XFIT/Koalariet XDD
-  RIET7/LHPM/CSRIET/ILL\_D1A5/PSI\_DMC DAT
-  Vamas ISO14976
   *(only experiment modes: SEM or MAPSV or MAPSVDP are supported; 
   only REGULAR scan_mode is supported)*
-  Princeton Instruments WinSpec SPE
   *(only 1-D data is supported)*
- χPLOT CHI_
- Ron Unwin's Spectra XPS format (VGX-900 compatible)

.. _CHI: http://www.esrf.eu/computing/scientific/FIT2D/FIT2D_REF/node115.html#SECTION0001851500000000000000

The library is written in C++, but it comes with C and Python bindings.

It also comes with a tiny program **xyconv**, which converts
supported formats to text with tab-separated values.

For API description, see `xylib/xylib.h`__ file.

__ https://raw.github.com/wojdyr/xylib/master/xylib/xylib.h

Licence: `LGPL <http://creativecommons.org/licenses/LGPL/2.1/>`_

DOWNLOAD
========

**Binary packages**:

* MS Windows: `xyconv.exe`_
* Linux: fresh RPMs and DEBs `from OBS`_
  http://software.opensuse.org/download/package?project=home:wojdyr&package=xylib

.. _`xyconv.exe`: http://downloads.sourceforge.net/xylib/xylib_win-1.1.zip
.. _`from OBS`: http://software.opensuse.org/download/package?project=home:wojdyr&package=xylib

**Source**:

* `tarball`_
* GitHub repository_ (requires ``autoreconf -i`` after cloning).

.. _`tarball`: http://downloads.sourceforge.net/xylib/xylib-1.1.tar.bz2
.. _repository: https://github.com/wojdyr/xylib

Prerequisites:

* C++ compiler (we tested GCC, MinGW, Visual C++)
* Boost_ libraries (only headers).
* optionally, zlib and bzlib libraries (for reading compressed files)

.. _Boost: http://www.boost.org/

On Unix, just type ``./configure && make``.

MISC NOTES
==========

The file `sample-urls`__ contains links to files in formats handled by xylib.

__ https://raw.github.com/wojdyr/xylib/master/sample-urls

In addition to C++ API, we provide C API and very simple `Python bindings`_.
So far we had no requests for binding to other languages.

.. _`Python bindings`: https://github.com/wojdyr/xylib/blob/master/xylib_capi.py

Documentation for programmers who want to extend xylib is
in the file `README.dev`__.

__ https://raw.github.com/wojdyr/xylib/master/README.dev

freecode__ provides new version notifications.

__ http://freecode.com/projects/xylib

xylib is used by:

-  `fityk <http://www.unipress.waw.pl/fityk>`_
-  `xyConvert <http://www.unipress.waw.pl/fityk/xyconvert>`_

AUTHORS
=======

-  Marcin Wojdyr wojdyr@gmail.com (maintainer)
-  Peng ZHANG zhangpengcas@gmail.com

CONTACT
=======

Feel free to send e-mail to the authors, or to the
`fityk-dev mailing list <http://groups.google.com/group/fityk-dev>`_.

CREDITS
=======

-  Google - the library was started as Google Summer of Code 2007 project
   by Peng ZHANG, mentored by Marcin Wojdyr from Fityk organization.
-  Michael Richardson provided VAMAS specification and sample files.
-  David Hovis provided a WinSpec file format specification and sample files.
-  Pablo Bianucci provided his code for reading WinSpec format and sample files.
-  Martijn Fransen provided very useful specifications of Philips formats.
-  Vincent Favre-Nicolin provided PSI\_DMC and ILL\_D1A5 samples;
   reading his ObjCryst library was also helpful.
-  Janos Vegh sent us his VAMAS reading routines (long time ago, before this
   project started).
-  Andreas Breslau added Bruker V3 (RAW1.01) support.
-  Bjørn Tore Lønstad provided Bruker RAW V3 format specification and samples.
-  Hector Zhao patched VAMAS code.
-  Jim Fitzgerald (author of FitzPeaks_) provided code for reading
   Canberra (Genie) CNF files.
-  Matthias Richter added Ron Unwin's Spectra XPS format.

.. _FitzPeaks: http://www.jimfitz.demon.co.uk/fitzpeak.htm

HISTORY
=======

* 1.1 (2012-11-05)

  - added XPS format from Ron Unwin's Spectra program (Matthias Richter)
  - fixed bug in reading energy calibration from Canberra formats

* 1.0 (2012-07-25)

  - added option ``decimal-comma`` for text format
  - fixed bug in CSV format

* 0.9 (2012-05-20)

  - added CSV format, or more acurately: delimiter-separated values format.
    Supports popular delimiters (``TAB ,;|:/``), quoting (``"``)
    and escape character (``\``). Non-numeric fields are read as NaNs.
  - added Canberra CNF format

* 0.8 (2011-01-18)

  - fixed a couple of bugs in pdCIF implementation

* 0.7 (2010-10-09)

  - added χPLOT (CHIPLOT) format (extension .chi)
  - fixed bug in reading VAMAS files with transmission data (Hector Zhao)

* 0.6 (2010-04-29)

  - fixed reading of Bruker v3 files
  - changes in API, added C API

* 0.5 (2010-01-04)

  - added support for compressed files \*.gz (requires zlib) and \*.bz2 (bzlib)

* 0.4 (2009-06-11)

  - added file caching (for details see ``xylib/cache.h``)
  - changes to parsing text files in which numeric data is mixed with text

* 0.3 (2008-06-03)

  - added Bruker binary RAW1.01 format
  - fixed bug in reading one-column ascii files

* 0.2 (2008-03-09)

  - initial public release

.. raw:: html

   <p align="right">
   <a href="http://sourceforge.net/projects/xylib">
   <img src="http://sflogo.sourceforge.net/sflogo.php?group_id=204287&amp;type=10" width="80" height="15" />
   </a>
   </p>

