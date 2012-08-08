#!/usr/bin/python
import sys
import os
import time
from optparse import OptionParser
from watermarker import Watermarker
from scipy import misc
from skimage.filter import tv_denoise

parser = OptionParser()
parser.add_option("-o", "--outfile", dest="outfile", default = None,
    help="output filename", metavar="FILE")
parser.add_option("-k", dest="k", type=int, default = 4,
    help="The k value to use in embedding (default is 4)")
parser.add_option("-s", dest="seed", type=int, default = 1895746671,
    help="The PRNG seed (if not given, a default one is used)")
parser.add_option("-m", dest="mother", type=str, default = "bior3.1",
    help="The mother wavelet (default is bior3.1)")
parser.add_option("-t", dest="tv_weight", type=int, default = 0,
    help="TV denoising weight (default is 0, meaning no denoising is performed)")
parser.add_option("-p", dest="payload_length", type=int, default = 6,
    help="Payload length (default is 6)")
parser.add_option("-e", dest="ecc_length", type=int, default = 4,
    help="ECC length (default is 4)")


if __name__ == "__main__":
    (options, args) = parser.parse_args()
    if len(args) != 2:
        parser.error("Usage: [options] <input image file> <payload>")
    infile, payload = args
    if not options.outfile:
        options.outfile = "%s-%s.png" % (os.path.basename(infile).split(".")[0], payload)
    
    t0 = time.time()
    w = Watermarker(options.payload_length, options.ecc_length, seed = options.seed, 
        mother = options.mother)
    out = w.embed(misc.imread(infile), payload, options.k)
    t1 = time.time()
    if options.tv_weight > 0:
        out = tv_denoise(out, options.tv_weight)
    
    misc.imsave(options.outfile, out)
    print "Created %s in %s seconds" % (options.outfile, t1-t0)


