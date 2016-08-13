#!/usr/bin/env python
"""
Simple, ctypes-based Python 2/3 interface to C API of xylib.

It would be possible to wrap it neatly in classes,
or to make bindings to C++ API of xylib,
but it is not planned due to low demand.
"""

from __future__ import print_function
from ctypes import cdll, c_char_p, c_double
import os

_dll_path = os.path.join(os.path.dirname(os.path.abspath(__file__)),
                         '_xylib.so')
if not os.path.exists(_dll_path):
    _dll_path = 'libxy.so.4'
lib = cdll.LoadLibrary(_dll_path)

get_version = lib.xylib_get_version
get_version.restype = c_char_p

load_file = lib.xylib_load_file

get_block = lib.xylib_get_block

count_columns = lib.xylib_count_columns

count_rows = lib.xylib_count_rows

get_data = lib.xylib_get_data
get_data.restype = c_double

dataset_metadata = lib.xylib_dataset_metadata
dataset_metadata.restype = c_char_p

block_metadata = lib.xylib_block_metadata
block_metadata.restype = c_char_p

free_dataset = lib.xylib_free_dataset


# Usage example. Can be tested as "python -m xylib my-file".

def _print_info(filename):
    if hasattr(filename, 'encode'):
        filename = filename.encode('utf-8')
    dataset = load_file(filename, None, None)
    if not dataset:
        sys.exit('File not found: %s' % filename)
    block = get_block(dataset, 0)

    ncol = count_columns(block)
    print('number of columns:', ncol)
    print('column lengths (-1 means a generator): ', end='')
    for i in range(ncol):
        print(count_rows(block, i+1), end='')

    n = min(count_rows(block, 2), 20)
    print('\ndata:',
          ' '.join('(%g, %g)' % (get_data(block, 1, i), get_data(block, 2, i))
                   for i in range(n)),
          '...')
    # metadata is file-format specific
    measure_date = dataset_metadata(dataset, 'MEASURE_DATE')
    if measure_date:
        print('measured at:', measure_date)
    used_lambda = block_metadata(block, 'USED_LAMBDA')
    if used_lambda:
        print('lambda:', used_lambda)

    free_dataset(dataset)


if __name__ == '__main__':
    import sys
    print('xylib version:', get_version())
    for arg in sys.argv[1:]:
        _print_info(arg)

