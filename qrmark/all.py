import numpy
from pywt import dwt2, idwt2
from scipy.stats import pearsonr
from scipy import misc, mean
from random import Random
from reedsolo import RSCodec, ReedSolomonError


def iterbits(data):
    for n in data:
        for i in (7,6,5,4,3,2,1,0):
            yield (n >> i) & 1

class Watermarker(object):
    EPSILON = 0.001
    def __init__(self, max_payload, ec_bytes, seed = 1910819922, mother = "haar"):
        self.mother = mother
        self.rscodec = RSCodec(ec_bytes)
        self.max_payload = max_payload
        self.total_bits = (max_payload + ec_bytes) * 8

        rand = Random(seed)
        chunk_size = 50000
        self.seq0 = numpy.array([int(rand.random() > 0.7) for _ in range(chunk_size)])
        self.seq1 = numpy.array(self.seq0[::-1])
        #while True:
        #    self.seq0 = numpy.array([int(rand.random() > 0.8) for _ in range(chunk_size)])
        #    self.seq1 = numpy.array([int(rand.random() > 0.75) for _ in range(chunk_size)])
        #    corr, _ = pearsonr(self.seq0, self.seq1)
        #    if corr < self.EPSILON:
        #        break
    
    def _embed(self, img, payload, k):
        cA, (cH, cV, cD) = dwt2(img, self.mother)
        cH2 = cH.reshape(cH.size)
        cV2 = cV.reshape(cV.size)
        cD2 = cD.reshape(cD.size)
        assert cH2.shape == cV2.shape == cD2.shape
        chunk_size = (cH2.size * 3) // (self.total_bits)
        sequence_of = (self.seq0[:chunk_size], self.seq1[:chunk_size])
        buffers = (cH2, cV2, cD2)
        
        for i, bit in enumerate(iterbits(payload)):
            seq = sequence_of[bit]
            target = buffers[i % 3]
            offset = (i//3) * chunk_size
            target[offset:offset + seq.size] += k * seq
        w, h = img.shape
        return idwt2((cA, (cH2.reshape(cH.shape), cV2.reshape(cV.shape), cD2.reshape(cD.shape))), self.mother)[:w,:h]
    
    def embed(self, img, payload, k):
        if len(payload) > self.max_payload:
            raise ValueError("payload too long")
        padded = bytearray(payload) + b"\x00" * (self.max_payload - len(payload))
        encoded = self.rscodec.encode(padded)
        
        if len(img.shape) == 2:
            return self._embed(img, encoded, k)
        elif len(img.shape) == 3:
            output = numpy.zeros(img.shape)
            for i in range(img.shape[2]):
                output[:,:,i] = self._embed(img[:,:,i], encoded, k)
            return output
        else:
            raise TypeError("img must be a 2d or 3d array")
    
    def _extract(self, img):
        cA, (cH, cV, cD) = dwt2(img, self.mother)
        cH2 = cH.reshape(cH.size)
        cV2 = cV.reshape(cV.size)
        cD2 = cD.reshape(cD.size)
        assert cH2.shape == cV2.shape == cD2.shape
        buffers = (cH2, cV2, cD2)
        chunk_size = (cH2.size * 3) // (self.total_bits)
        seq0 = self.seq0[:chunk_size]
        seq1 = self.seq1[:chunk_size]

        byte = 0
        output = bytearray()
        for i in range(self.total_bits):
            target = buffers[i % 3]
            offset = (i//3) * chunk_size
            chunk = target[offset : offset + seq0.size]
            corr0, _ = pearsonr(chunk, seq0)
            corr1, _ = pearsonr(chunk, seq1)
            bit = int(corr1 > corr0)
            byte = (byte << 1) | bit
            if i % 8 == 7:
                output.append(byte)
                byte = 0
        #print repr(output)
        return output
    
    def extract(self, img):
        if len(img.shape) == 2:
            return self.rscodec.decode(self._extract(img))
        elif len(img.shape) == 3:
            for i in range(img.shape[2]):
                try:
                    return self.rscodec.decode(self._extract(img[:,:,i]))
                except ReedSolomonError:
                    pass
            return self.rscodec.decode(self._extract(mean(img, 2)))
        else:
            raise TypeError("img must be a 2d or 3d array")

def test_jpg(w, img):
    prev_i = 100
    for i in range(prev_i, 0, -5):
        misc.toimage(img).save("out.jpg", quality = i)
        try:
            w.extract(misc.imread("out.jpg"))
        except ReedSolomonError:
            return prev_i
        prev_i = i
    return 5

def test_filter(w, img, filterfunc):
    prev_i = 0
    for i in range(0, 100):
        print i
        misc.imsave("out.png", filterfunc(img, i))
        try:
            w.extract(misc.imread("out.png"))
        except ReedSolomonError:
            return prev_i
        prev_i = i

def test_recursive_filter(w, img, filterfunc):
    prev_i = 0
    misc.imsave("out.png", out)
    for i in range(0, 20):
        print i, 
        misc.imsave("out.png", filterfunc(misc.imread("out.png")))
        try:
            w.extract(misc.imread("out.png"))
        except ReedSolomonError:
            return prev_i
        prev_i = i
    return prev_i


if __name__ == "__main__":
    from scipy.ndimage.filters import gaussian_filter
    from skimage.filter import tv_denoise

    w = Watermarker(6, 3, 2394181, "bior3.3")
    out = w.embed(misc.imread("pics/lena.png"), "123456", 10)
    misc.imsave("out3.png", out)
    exit()
    #test_filter(w, out, lambda img, i: gaussian_filter(img, i / 10.0))
    #test_filter(w, out, tv_denoise)
    #print test_recursive_filter(w, out, lambda img: misc.imfilter(img, "smooth"))
    
    for flt in ["blur", "contour", "detail", "edge_enhance", "edge_enhance_more", "emboss", "find_edges", "smooth", "smooth_more", "sharpen"]:
        print flt, 
        print test_recursive_filter(w, out, lambda img: misc.imfilter(img, flt))
        print







