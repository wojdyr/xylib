#!/bin/sh

autoreconf --install --verbose  \
&& ./configure "$@"

