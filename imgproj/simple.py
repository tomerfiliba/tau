import os
import numpy
from scipy import misc
from watermarker import Watermarker
from test import test_jpg, add_blocks, add_noise, test_filter, test_recursive_filter
from scipy.ndimage.filters import gaussian_filter, gaussian_laplace, uniform_filter, median_filter
from skimage.filter import tv_denoise


src = misc.lena()
markers = [Watermarker(6, 4, seed = seed) for seed in range(50)]
text = "the quick brown fox jumped over the lazy dog and laughed a great deal"

for i, w in enumerate(markers):    
    src = w.embed(src, text[i:i+6])
    print "%s\t%s" % (i+1, test_jpg(markers[0], src))


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
