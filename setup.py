#!/usr/bin/env python

print('''
This setup.py builds xylib as a python extension, which is experimental.
The normal way of building xylib is using configure && make. Or cmake.
''')


from distutils.core import setup, Extension
from distutils.command.sdist import sdist
from glob import glob
import os

sources = glob(os.path.join('xylib/*.cpp'))
headers = glob(os.path.join('xylib/*.h'))

# from distutils/command/sdist.py, the default files are, among others:
#       - all C sources listed as part of extensions or C libraries
#         in the setup script (doesn't catch C headers!)
# No idea what was the reason to have C sources without headers.
# Anyway, let's monkey-patch it.
sdist.add_defaults_without_headers = sdist.add_defaults
def add_defaults_with_headers(self):
    self.add_defaults_without_headers()
    self.filelist.extend(self.distribution.headers)
sdist.add_defaults = add_defaults_with_headers

setup(name='xylib-py',
      version='0.0.1',
      description='Python bindings to xylib',
      author='Marcin Wojdyr',
      author_email='wojdyr@gmail.com',
      license= 'LGPL2.1',
      url='https://github.com/wojdyr/xylib',
      ext_modules=[Extension('_xylib',
                             sources=sources,
                             language='c++',
                             include_dirs=['.'],
                             libraries=[])],
      #package_dir={'': 'xylib'},
      py_modules=['xylib'],
      headers=headers)
