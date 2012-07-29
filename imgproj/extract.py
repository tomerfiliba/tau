#!/usr/bin/python
import sys
import os
from watermarker import Watermarker
from scipy import misc

if __name__ == "__main__":
    if len(sys.argv) != 2:
        parser.error("Usage: <input file>")
    w = Watermarker(6, 4)
    print w.extract(misc.imread(sys.argv[1]))


