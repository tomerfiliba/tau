#!/usr/bin/python
import sys
import os
import time
from watermarker import Watermarker
from scipy import misc
from optparse import OptionParser

parser = OptionParser()
parser.add_option("-s", dest="seed", type=int, default = 1895746671,
    help="The PRNG seed (if not given, a default one is used)")
parser.add_option("-m", dest="mother", type=str, default = "bior3.1",
    help="The mother wavelet (default is bior3.1)")
parser.add_option("-p", dest="payload_length", type=int, default = 6,
    help="Payload length (default is 6)")
parser.add_option("-e", dest="ecc_length", type=int, default = 4,
    help="ECC length (default is 4)")


if __name__ == "__main__":
    (options, args) = parser.parse_args()
    if len(args) != 1:
        parser.error("Usage: [options] <input image file>")
    w = Watermarker(options.payload_length, options.ecc_length, seed = options.seed, 
        mother = options.mother)
    src = misc.imread(sys.argv[1])
    
    t0 = time.time()
    print w.extract(src)
    t1 = time.time()
    
    print "Extraction took %s" % (t1 - t0,)

