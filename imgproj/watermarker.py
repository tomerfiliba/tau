import numpy
from pywt import dwt2, idwt2
from scipy.stats import pearsonr
from scipy import mean
from scipy.misc import toimage
from random import Random
from reedsolo import RSCodec, ReedSolomonError
from skimage.exposure import rescale_intensity


def iterbits(data):
    if isinstance(data, str):
        data = [ord(ch) for ch in data]
    for n in data:
        for i in (7,6,5,4,3,2,1,0):
            yield (n >> i) & 1

class Watermarker(object):
    def __init__(self, max_payload, ec_bytes, seed = 1895746671, mother = "bior3.1", sparsity = 0.7):
        self.mother = mother
        self.sparsity = sparsity
        self.rscodec = RSCodec(ec_bytes)
        self.max_payload = max_payload
        self.total_bits = (max_payload + ec_bytes) * 8
        self.seed = seed
    
    @classmethod
    def _interleave(cls, cH, cV, cD):
        vec = numpy.zeros(cH.size + cV.size + cD.size, dtype = float)
        vec[0::3] = cH.ravel()
        vec[1::3] = cV.ravel()
        vec[2::3] = cD.ravel()
        return vec
    
    @classmethod
    def _deinterleave(cls, vec, cH, cV, cD):
        return vec[0::3].reshape(cH.shape), vec[1::3].reshape(cV.shape), vec[2::3].reshape(cD.shape)
    
    def _generate_sequences(self, chunk_size):
        rand = Random(self.seed)
        seq0 = numpy.array([int(rand.random() >= self.sparsity) for _ in range(chunk_size)])
        seq1 = seq0[::-1]
        return seq0, seq1
    
    def _embed(self, img, payload, k):
        cA, (cH, cV, cD) = dwt2(img.astype(float), self.mother)
        vec = self._interleave(cH, cV, cD)
        chunk_size = vec.size // self.total_bits
        sequences = self._generate_sequences(chunk_size)
        
        for i, bit in enumerate(iterbits(payload)):
            offset = i * chunk_size
            vec[offset : offset + chunk_size] += k * sequences[bit]
            #vec[i : self.total_bits*chunk_size : self.total_bits] += k * sequences[bit]
        
        w, h = img.shape
        cH2, cV2, cD2 = self._deinterleave(vec, cH, cV, cD)
        return idwt2((cA, (cH2, cV2, cD2)), self.mother)[:w,:h]
    
    def embed(self, img, payload, k = 4, rescale_color = True):
        if len(payload) > self.max_payload:
            raise ValueError("payload too long")
        padded = bytearray(payload) + b"\x00" * (self.max_payload - len(payload))
        encoded = self.rscodec.encode(padded)
        
        if img.ndim == 2:
            output = self._embed(img, encoded, k)
        elif img.ndim == 3:
            output = numpy.zeros(img.shape, dtype=float)
            for i in range(img.shape[2]):
                output[:,:,i] = self._embed(img[:,:,i], encoded, k)
        else:
            raise TypeError("img must be a 2d or 3d array")
        
        if rescale_color:
            output = rescale_intensity(output, out_range = (numpy.min(img), numpy.max(img)))
        return output
    
    def _extract(self, img):
        cA, (cH, cV, cD) = dwt2(img.astype(float), self.mother)
        vec = self._interleave(cH, cV, cD)
        chunk_size = vec.size // self.total_bits
        seq0, seq1 = self._generate_sequences(chunk_size)

        byte = 0
        output = bytearray()
        for i in range(self.total_bits):
            offset = i * chunk_size
            chunk = vec[offset: offset + chunk_size]
            #chunk = vec[i:self.total_bits*chunk_size:self.total_bits]
            corr0, _ = pearsonr(chunk, seq0)
            corr1, _ = pearsonr(chunk, seq1)
            bit = int(corr1 > corr0)
            byte = (byte << 1) | bit
            if i % 8 == 7:
                output.append(byte)
                byte = 0
        
        return output
    
    def _try_decode(self, payload):
        try:
            return self.rscodec.decode(payload)
        except ReedSolomonError:
            rpayload = bytearray(b ^ 255 for b in payload)
            return self.rscodec.decode(rpayload)
    
    def extract(self, img):
        if img.ndim == 2:
            return self._try_decode(self._extract(img))
        elif img.ndim == 3:
            for i in range(img.shape[2]):
                try:
                    return self._try_decode(self._extract(img[:,:,i]))
                except ReedSolomonError:
                    pass
            return self._try_decode(self._extract(mean(img, 2)))
        else:
            raise TypeError("img must be a 2d or 3d array")


