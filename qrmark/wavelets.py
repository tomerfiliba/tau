import numpy
import pywt
import math
from scipy import misc
from random import Random

rand = Random()


def corr2(A, B):
    # http://www.mathworks.com/help/toolbox/images/ref/corr2.html
    meanA = numpy.mean(A)
    meanB = numpy.mean(B)
    W = range(min(A.shape[0], B.shape[0]))
    H = range(min(A.shape[1], B.shape[1]))
    x = sum((A[i,j] - meanA) * (B[i,j] - meanB) for i in W for j in H)
    y = sum((A[i,j] - meanA) ** 2 for i in W for j in H)
    z = sum((B[i,j] - meanB) ** 2 for i in W for j in H)
    return x / math.sqrt(y * z)

def rand_array(m, n):
    arr = numpy.zeros((m, n))
    for i in range(m):
        for j in range(n):
            arr[i, j] = rand.choice([0, 1])
    return arr

SEED = 179823472
def embed(host, payload, k = 2, seed = SEED):
    if host.dtype != numpy.float64:
        host = host.astype(numpy.float64)
    w = host.shape[0] // 2
    h = host.shape[1] // 2
    cA, (cH, cV, cD) = pywt.dwt2(host, "haar")
    rand.seed(seed)
    for bit in payload:
        pn_sequence_h = rand_array(w, h);
        pn_sequence_v = rand_array(w, h);
        if bit:
            cH += k * pn_sequence_h
            cV += k * pn_sequence_h
    
    result = pywt.idwt2((cA, (cH, cV, cD)), "haar")
    return result

def extract(watermarked, length, seed = SEED):
    if watermarked.dtype != numpy.float64:
        watermarked = watermarked.astype(numpy.float64)
    print watermarked.shape
    payload = []
    w = watermarked.shape[0] // 2
    h = watermarked.shape[1] // 2
    cA, (cH, cV, cD) = pywt.dwt2(watermarked, "haar")
    corr_vector = []
    rand.seed(seed)
    for i in range(length):
        print i
        pn_sequence_h = rand_array(w, h);
        pn_sequence_v = rand_array(w, h);
        corr_h = corr2(cH, pn_sequence_h);
        corr_v = corr2(cV, pn_sequence_v);
        corr_vector.append((corr_h + corr_v) / 2.0)
    
    mean = sum(corr_vector) / len(corr_vector)
    for corr in corr_vector:
        payload.append(1 if corr > mean else 0)
    return payload


if __name__ == "__main__":
    #host = misc.lena()
    payload = [0, 0, 0, 1, 1, 1] * 4
    #watermarked_image = embed(host, payload)
    #misc.imsave("out.png", watermarked_image)
    #misc.imsave("out.jpg", watermarked_image)
    #print "embedded"

    #print "ORIG", extract(watermarked_image, len(payload))
    #print "PNG ", extract(misc.imread("out.png"), len(payload))
    #print "JPG ", extract(misc.imread("out.jpg"), len(payload))
    #print "CAP ", extract(misc.imread("Untitled3.bmp"), len(payload))
    #pic = misc.imread("out2.jpg")[:,:,0]
    #print "ROT", extract(pic, len(payload))
    #print "SCALE", extract(misc.imread("out3.jpg"), len(payload))




