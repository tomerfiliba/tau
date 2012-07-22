import numpy
import math
from pywt import dwt2, idwt2
import scipy
from scipy.fftpack import fft2, ifft2, fftshift, ifftshift, dct, idct
from scipy.stats import pearsonr
from scipy import misc, mean
from random import Random
from reedsolo import RSCodec, ReedSolomonError


class Aligner(object):
    def __init__(self, seed, seq_size = (128, 128), mother = "haar"):
        self.mother = mother
        rand = Random(seed)
        self.seq = numpy.array([[int(rand.random() > 0.6) for _ in range(seq_size[1])] 
            for _ in range(seq_size[0])])
        #self.seq = numpy.array([int(rand.random() > 0.6) for _ in range(seq_size[0]*seq_size[1])])

    def embed_alignment(self, img, k ):
        freq = dct(img.astype(float), norm='ortho')
        freq[:self.seq.shape[0],:self.seq.shape[1]] += k * self.seq
        return idct(freq, norm='ortho')
    
    def _corr(self, img):
        freq = dct(img.astype(float), norm='ortho')
        chunk = freq[:self.seq.shape[0],:self.seq.shape[1]]
        corr, _ = pearsonr(chunk.reshape(chunk.size), self.seq.reshape(self.seq.size))
        return corr
    
    def correct_rotation(self, img, amin, amax, step = 1):
        results = []
        for a in range(int(amin / step), int(amax / step) + 1):
            rot = misc.imrotate(img, a * step)
            corr = self._corr(rot)
            print a * step, corr
            results.append((corr, rot))
        return max(results, key = lambda x: x[0])[1]


def expand_rotate(img, angle):
    return numpy.array(misc.toimage(img).rotate(angle, expand = True))


if __name__ == "__main__":
    algn = Aligner(3489711, mother = "bior2.2")
    img = algn.embed_alignment(misc.lena(), 100)
    misc.imsave("rot.png", img)
    print algn._corr(img)
    
    #img2 = expand_rotate(img, 7)
    
    #misc.imsave("unrot.png", algn.correct_rotation(img2, 0, 0))



















