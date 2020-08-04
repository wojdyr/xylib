#!/usr/bin/env python

# This setup.py builds xylib as a python extension, which is experimental.
# The normal way of building xylib is using configure && make. Or cmake.
long_description="""\
This package is a collection of built wheels for xylib-py (https://pypi.org/project/xylib-py/), pending approval of a PR that would autoupload these wheels to that package.

Original Description:
xylib is a library for reading obscure file formats with data from
powder diffraction, spectroscopy and other experimental methods.
For the list of supported formats see https://github.com/wojdyr/xylib .

This module includes bindings to xylib and xylib itself.
The first two numbers in the version are the version of included xylib.
"""
from setuptools import setup
from distutils.core import Extension
from distutils.command.sdist import sdist
from glob import glob
import sys

# as per http://stackoverflow.com/a/21236111/104453
from distutils.command.build import build
class CustomBuild(build):
    sub_commands = [('build_ext', build.has_ext_modules),
                    ('build_py', build.has_pure_modules),
                    ('build_clib', build.has_c_libraries),
                    ('build_scripts', build.has_scripts)]


# from distutils/command/sdist.py, the default files are, among others:
#       - all C sources listed as part of extensions or C libraries
#         in the setup script (doesn't catch C headers!)
# No idea what was the reason to have C sources without headers.
# Anyway, let's monkey-patch it.
sdist.add_defaults_without_headers = sdist.add_defaults
def add_defaults_with_headers(self):
    self.add_defaults_without_headers()
    self.filelist.extend(self.distribution.headers)
    # actually we may add also a few other files, no need for MANIFEST.in
    self.filelist.extend(['README.rst', 'xyconv.py'])
sdist.add_defaults = add_defaults_with_headers


sources = glob('xylib/*.cpp') + ['xylib.i']
headers = glob('xylib/*.h')

swig_opts = ['-c++', '-modern', '-modernargs']
if sys.version_info[0] == 3:
    swig_opts += ['-py3']

setup(name='xylib-py-wheels',
      version='1.6.0',
      description='Built wheels for xylib-py by Marcin Wojdyr',
      long_description=long_description,
      classifiers=[
          'Development Status :: 5 - Production/Stable',
          'Intended Audience :: Developers',
          'Intended Audience :: Science/Research',
          'Programming Language :: Python :: Implementation :: CPython',
          'Topic :: Scientific/Engineering :: Chemistry',
          'Topic :: Software Development :: Libraries :: Python Modules',
          ],
      author='Christopher Stallard',
      author_email='christopher.stallard1@gmail.com',
      license='LGPL2.1',
      url='https://github.com/chris-stallard1/xylib',
      ext_modules=[Extension('_xylib',
                             sources=sources,
                             language='c++',
                             swig_opts=swig_opts,
                             include_dirs=['.'],
                             libraries=[])],
      py_modules=['xylib'],
      headers=headers,
      cmdclass={'build': CustomBuild})
