import numpy
from pywt import dwt2, idwt2
from scipy.stats import pearsonr
from scipy import mean
from scipy.misc import toimage
from random import Random
from reedsolo import RSCodec, ReedSolomonError
from skimage.filter import tv_denoise
from skimage.exposure import rescale_intensity


def iterbits(data):
    if isinstance(data, str):
        data = [ord(ch) for ch in data]
    for n in data:
        for i in (7,6,5,4,3,2,1,0):
            yield (n >> i) & 1

class Watermarker(object):
    def __init__(self, max_payload, ec_bytes, seed = 1895746671, mother = "bior3.3"):
        self.mother = mother
        self.rscodec = RSCodec(ec_bytes)
        self.max_payload = max_payload
        self.total_bits = (max_payload + ec_bytes) * 8
        self.seed = seed
    
    @classmethod
    def _interleave(cls, cH, cV, cD):
        arr = numpy.zeros(cH.size + cV.size + cD.size)
        sources = [cH.ravel(), cV.ravel(), cD.ravel()]
        for i in range(arr.size):
            src = sources[i % 3]
            j = i // 3
            if j >= src.size:
                arr = arr[:i]
                break
            arr[i] = src[j]
        return arr
    
    @classmethod
    def _deinterleave(cls, arr, cH, cV, cD):
        cH2 = numpy.zeros(cH.size)
        cV2 = numpy.zeros(cV.size)
        cD2 = numpy.zeros(cD.size)
        destinations = [cH2, cV2, cD2]
        for i in range(arr.size):
            destinations[i % 3][i // 3] = arr[i]
        return cH2.reshape(cH.shape), cV2.reshape(cV.shape), cD2.reshape(cD.shape)
    
    def _generate_sequences(self, chunk_size):
        rand = Random(self.seed)
        seq0 = numpy.array([int(rand.random() > 0.7) for _ in range(chunk_size)])
        seq1 = seq0[::-1]
        return seq0, seq1
    
    def _embed(self, img, payload, k):
        cA, (cH, cV, cD) = dwt2(img, self.mother)
        arr = self._interleave(cH, cV, cD)
        chunk_size = arr.size // self.total_bits
        sequences = self._generate_sequences(chunk_size)
        
        for i, bit in enumerate(iterbits(payload)):
            offset = i * chunk_size
            arr[offset : offset + chunk_size] += k * sequences[bit]
        
        w, h = img.shape
        cH2, cV2, cD2 = self._deinterleave(arr, cH, cV, cD)
        return idwt2((cA, (cH2, cV2, cD2)), self.mother)[:w,:h]
    
    def embed(self, img, payload, k = 6, tv_denoising_weight = 4, rescale = True):
        if len(payload) > self.max_payload:
            raise ValueError("payload too long")
        padded = bytearray(payload) + b"\x00" * (self.max_payload - len(payload))
        encoded = self.rscodec.encode(padded)
        
        if img.ndim == 2:
            output = self._embed(img, encoded, k)
        elif img.ndim == 3:
            output = numpy.zeros(img.shape)
            for i in range(img.shape[2]):
                output[:,:,i] = self._embed(img[:,:,i], encoded, k)
        else:
            raise TypeError("img must be a 2d or 3d array")
        
        if tv_denoising_weight > 0:
            output = tv_denoise(output, tv_denoising_weight)
        if rescale:
            output = rescale_intensity(output, out_range = (numpy.min(img), numpy.max(img)))
            #if img.ndim == 2:
            #elif img.ndim == 3:
            #    for i in range(img.shape[2]):
            #        output[:,:,i] = rescale_intensity(output[:,:,i], 
            #            out_range = (numpy.min(img[:,:,i]), numpy.max(img[:,:,i])))
        return toimage(output,cmin=0,cmax=255)
    
    def _extract(self, img):
        cA, (cH, cV, cD) = dwt2(img, self.mother)
        arr = self._interleave(cH, cV, cD)
        chunk_size = arr.size // self.total_bits
        seq0, seq1 = self._generate_sequences(chunk_size)

        byte = 0
        output = bytearray()
        for i in range(self.total_bits):
            offset = i * chunk_size
            chunk = arr[offset: offset + chunk_size]
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


