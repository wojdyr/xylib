#!/usr/bin/env python
# Simplified, pythonic version of xyconv - a test for xylib bindings
# Licence: Lesser GNU Public License 2.1 (LGPL)

import argparse
import sys
import xylib

__version__ = xylib.xylib_get_version()

def list_supported_formats():
    n = 0
    while True:
        form = xylib.xylib_get_format(n)
        if not form:
            break
        print('%-20s: %s' % (form.name, form.desc))
        n += 1

def print_filetype_info(filetype):
    fi = xylib.xylib_get_format_by_name(filetype)
    if fi:
        print("Name: %s" % fi.name)
        print("Description: %s" % fi.desc)
        print("Possible extensions: %s" % (fi.exts or "(not specified)"))
        print("Flags: %s-file %s-block" % (
            ("binary" if fi.binary else "text"),
            ("multi" if fi.multiblock else "single")))
        print("Options: %s" % (fi.valid_options or '-'))
    else:
        print("Unknown file format.")


def export_metadata(f, meta):
    for i in range(meta.size()):
        key = meta.get_key(i)
        value = meta.get(key)
        f.write('# %s: %s\n' % (key, value.replace('\n', '\n#\t')))


def convert_file(opt):
    if opt.INPUT_FILE == '-':
        src = (sys.stdin.buffer if hasattr(sys.stdin, 'buffer') else sys.stdin)
        d = xylib.load_string(src.read(), opt.t)
    else:
        d = xylib.load_file(opt.INPUT_FILE, opt.t or '')
    f = opt.OUTPUT_FILE
    f.write('# exported by xylib from a %s file\n' % d.fi.name)
    # output the file-level meta-info
    if not opt.s and d.meta.size():
        export_metadata(f, d.meta)
        f.write('\n')
    nb = d.get_block_count()
    for i in range(nb):
        block = d.get_block(i)
        if nb > 1 or block.get_name():
            f.write('\n### block #%d %s\n', i, block.get_name())
        if not opt.s:
            export_metadata(f, block.meta)

        ncol = block.get_column_count()
        # column 0 is pseudo-column with point indices, we skip it
        col_names = [block.get_column(k).get_name() or ('column_%d' % k)
                     for k in range(1, ncol+1)]
        f.write('# ' + '\t'.join(col_names) + '\n')
        nrow = block.get_point_count()
        for j in range(nrow):
            values = ["%.6f" % block.get_column(k).get_value(j)
                      for k in range(1, ncol+1)]
            f.write('\t'.join(values) + '\n')



def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--version', action='version', version=__version__)
    parser.add_argument('-l', action='store_true',
                        help='list supported file types')
    parser.add_argument('-i', metavar='FILETYPE',
                        help='show information about filetype')
    parser.add_argument('-s', action='store_true',
                        help='do not output metadata')
    parser.add_argument('-t', metavar='TYPE',
                        help='specify filetype of input file')
    parser.add_argument('INPUT_FILE', nargs='?')
    parser.add_argument('OUTPUT_FILE', type=argparse.FileType('w'), nargs='?')
    opt = parser.parse_args()
    if opt.l:
        list_supported_formats()
        return
    if opt.i:
        print_filetype_info(opt.i)
        return
    if not opt.INPUT_FILE or not opt.OUTPUT_FILE:
        sys.exit('Specify input and output files. Or "-h" for details.')
    if opt.INPUT_FILE == '-' and not opt.t:
        sys.exit('You need to specify file format for stdin input')
    try:
        convert_file(opt)
    except RuntimeError as e:
        sys.exit(str(e))

if __name__ == '__main__':
    main()
