
%module xylib

%{
#define BUILDING_XYLIB
#include "xylib/xylib.h"
%}
%include "std_string.i"
%include "std_except.i"

%include "xylib/xylib.h"
