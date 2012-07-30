#!/usr/bin/python
import sys
import os
from optparse import OptionParser
from watermarker import Watermarker
from scipy import misc

parser = OptionParser()
parser.add_option("-o", "--outfile", dest="outfile", default = None,
    help="output filename", metavar="FILE")
parser.add_option("-k", dest="k", type=int, default=6,
    help="The k value to use in embedding (default is 6)")

if __name__ == "__main__":
    (options, args) = parser.parse_args()
    if len(args) != 2:
        parser.error("Usage: <input file> <payload>")
    infile, payload = args
    if len(payload) > 6:
        parser.error("Payload too long (up to 6 bytes)")
    if not options.outfile:
        options.outfile = "%s-%s.png" % (os.path.basename(infile).split(".")[0], payload)
    w = Watermarker(6, 4)
    misc.imsave(options.outfile, w.embed(misc.imread(infile), payload, options.k))
    print "Created %s" % (options.outfile,)


