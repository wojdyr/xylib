
%module xylib

%{
#define BUILDING_XYLIB
#define SWIG_FILE_WITH_INIT
#include "xylib/xylib.h"
%}
%include "std_string.i"
%include "std_except.i"

/* possible improvements:
 *  - better __str__/__repr__
 *  - better exception handling
*/

%catches(std::runtime_error) load_file(std::string const& path,
                                       std::string const& format_name="",
                                       std::string const& options="");
%catches(std::runtime_error) load_string(std::string const& str,
                                         std::string const& format_name,
                                         std::string const& options="");

#if defined(SWIGPYTHON)
// istream is not wrapped automatically
%ignore load_stream;
%ignore guess_filetype;
%ignore check_format;
#endif

%include "xylib/xylib.h"
