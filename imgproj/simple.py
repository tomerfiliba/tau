import numpy
from scipy import misc
from watermarker import Watermarker
from test import test_jpg, add_blocks, test_filter
from skimage.exposure import rescale_intensity

from pywt import dwt2, idwt2
cA, (cH, cV, cD) = dwt2(misc.lena(), "haar")
z = numpy.ones(cA.shape)*255
out = idwt2((z, (cH, cV, cD)), "haar")
misc.imsave("out.png", out)

exit()

w = Watermarker(6, 4)
#src = misc.imread("../qrmark/pics/sky.png")
src = misc.imread("pics/face.jpg")
#print numpy.min(src), numpy.mean(src), numpy.median(src), numpy.max(src)
out = w.embed(src, "123456", k = 2, tv_denoising_weight = 0)
#print numpy.min(out), numpy.mean(out), numpy.median(out), numpy.max(out)
misc.imsave("orig.png", out)

out2 = misc.imread("orig.png")
print w.extract(out2)
print "min jpg quality:", test_jpg(w, out2)

print "Max random block coverage: ", test_filter(w, out2, "blocks-%s.png", 
    lambda img, k: add_blocks(img, k, 50, 50), [i/20.0 for i in range(1, 20)])
