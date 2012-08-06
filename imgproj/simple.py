import numpy
from scipy import misc
from watermarker import Watermarker
from test import test_jpg, add_blocks, test_filter
from skimage.exposure import rescale_intensity
import pywt

data = "the quick brown fox jumped over the lazy dog"

for i in range(5, 32, 3):
    for ecc in range(2, i+1, 3):
        w = Watermarker(i, ecc)
        src = misc.lena()
        out = w.embed(src, data[:i])
        misc.imsave("orig.png", out)
        out2 = misc.imread("orig.png")
        print "%s\t%s\t%s" % (i, ecc, test_jpg(w, out2),)

#out = w2.embed(out, "789012", k = 4, tv_denoising_weight = 0)
#out2 = misc.imread("orig.png")
#out2 = misc.imread("facescan.png")
#print w.extract(out2)
#print "min jpg quality:", test_jpg(w, out2)

#print w2.extract(out2)
#print "min jpg quality:", test_jpg(w2, out2)

#import random
#random.seed(3286912)
#print "Max random block coverage: ", test_filter(w, out2, "blocks-%s.png", 
#    lambda img, k: add_blocks(img, k, 50, 50), [i/20.0 for i in range(1, 20)])
