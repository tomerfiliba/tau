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
        chunk_size = 10000
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
        #assert cH2.shape == cV2.shape == cD2.shape
        buffer = numpy.zeros(cH.size + cV.size + cD.size)
        i = 0
        for arr in (cH, cV, cD):
            for row in arr:
                buffer[i:i+row.size] = row
                i += row.size
        chunk_size = buffer.size // self.total_bits
        sequence_of = (self.seq0[:chunk_size], self.seq1[:chunk_size])
        
        for i, bit in enumerate(iterbits(payload)):
            seq = sequence_of[bit]
            buffer[i * chunk_size : i * chunk_size + seq.size] += k * seq
        
        detail = (numpy.zeros(cH.shape), numpy.zeros(cV.shape), numpy.zeros(cD.shape))
        i = 0
        for arr in detail:
            h, w = arr.shape
            for r in range(h):
                arr[r] = buffer[i:i+w]
                i += w

        h, w = img.shape
        return idwt2((cA, detail), self.mother)[:h,:w]
    
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
        chunk_size = cH2.size // (self.total_bits // 3)
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

        
if __name__ == "__main__":
    w = Watermarker(6, 3, 2394181, "bior3.3")
    out = w.embed(misc.imread("pics/face.jpg"), "123456", 7)
    misc.imsave("out.png", out)
    #for i in range(100, 0, -5):
    #    print i, 
    #    misc.toimage(out).save("out.jpg", quality = i)
    #    print w.extract(misc.imread("out.jpg"))







