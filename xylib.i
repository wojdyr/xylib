
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
%catches(std::runtime_error) load_string(std::string const& buffer,
                                         std::string const& format_name,
                                         std::string const& options="");

#if defined(SWIGPYTHON)
// istream is not wrapped automatically
%ignore load_stream;
%ignore guess_filetype;
%ignore check_format;

%#if PY_VERSION_HEX >= 0x03000000
// buffer in load_string() must be mapped to bytes not string
%typemap(typecheck) std::string const& buffer %{
    $1 = PyBytes_Check($input) ? 1 : 0;
%}
%typemap(in) std::string const& buffer (std::string temp) {
    char *cstr;
    Py_ssize_t len;
    if (PyBytes_AsStringAndSize($input, &cstr, &len) < 0) SWIG_fail;
    temp.assign(cstr, len);
    $1 = &temp;
}
// delete default freearg typemap that checks res$argnum
%typemap(freearg) std::string const& buffer "";

%#endif // python3
#endif // SWIGPYTHON

%include "xylib/xylib.h"
