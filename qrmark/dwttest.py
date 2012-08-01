import numpy
from scipy import misc
from scipy.fftpack import dct, idct, fftshift
from pywt import dwt2, idwt2

img = misc.toimage(misc.imread("pics/cat.jpg"))
img2 = img

arr = numpy.fromstring(img.convert('YCbCr').tostring(), dtype=numpy.uint8).reshape((img.size[0], img.size[1], 3))
misc.imsave("out.png", arr)
#print numpy.array(img2, )
exit()


cA, (cH, cV, cD) = dwt2(misc.lena(), "bior3.1")
#z = numpy.zeros(cH.shape)
cH[::2] = 0
cV[::2] = 0
cD[::2] = 0
out = idwt2((cA, (cH, cV, cD)), "bior3.1")
#out = dct(misc.lena().astype(float))
#out = fftshift(out)

misc.imsave("out.png", out)


