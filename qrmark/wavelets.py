import numpy
from scipy import misc
import pywt


def corr2(A, B):
    # http://www.mathworks.com/help/toolbox/images/ref/corr2.html
    meanA = numpy.mean(A)
    meanB = numpy.mean(B)
    x = sum((A[i,j] - meanA) * (B[i,j] - meanB) for i in range(A.shape[0]) for j in range(A.shape[1]))
    y = sum((A[i,j] - meanA) ** 2 for i in range(A.shape[0]) for j in range(A.shape[1]))
    z = sum((B[i,j] - meanB) ** 2 for i in range(A.shape[0]) for j in range(A.shape[1]))
    return x / math.sqrt(y * z)

host = misc.lena().astype(numpy.float64)
w, h = host.shape
cA, (cH, cV, cD) = pywt.dwt2(host, "haar")






