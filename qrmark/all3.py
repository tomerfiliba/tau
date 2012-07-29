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
    
    def _interleave(self, cH, cV, cD):
        arr = numpy.zeros(cH.size + cV.size + cD.size)
        sources = [cH.reshape(cH.size), cV.reshape(cV.size), cD.reshape(cD.size)]
        for i in range(arr.size):
            src = sources[i % 3]
            j = i // 3
            if j >= src.size:
                arr = arr[:i]
                break
            arr[i] = src[j]
        return arr
    
    def _deinterleave(self, arr, cH, cV, cD):
        cH2 = numpy.zeros(cH.size)
        cV2 = numpy.zeros(cV.size)
        cD2 = numpy.zeros(cD.size)
        destinations = [cH2, cV2, cD2]
        for i in range(arr.size):
            destinations[i % 3][i // 3] = arr[i]
        return cH2.reshape(cH.shape), cV2.reshape(cV.shape), cD2.reshape(cD.shape)
    
    def _embed(self, img, payload, k):
        cA, (cH, cV, cD) = dwt2(img, self.mother)
        arr = self._interleave(cH, cV, cD)
        chunk_size = arr.size // self.total_bits
        sequence_of = (self.seq0[:chunk_size], self.seq1[:chunk_size])
        
        for i, bit in enumerate(iterbits(payload)):
            seq = sequence_of[bit]
            arr[i * chunk_size: i * chunk_size + seq.size] += k * seq
        
        w, h = img.shape
        cH2, cV2, cD2 = self._deinterleave(arr, cH, cV, cD)
        return idwt2((cA, (cH2, cV2, cD2)), self.mother)[:w,:h]
    
    def embed(self, img, payload, k):
        if len(payload) > self.max_payload:
            raise ValueError("payload too long")
        padded = bytearray(payload) + b"\x00" * (self.max_payload - len(payload))
        encoded = self.rscodec.encode(padded)
        
        if img.ndim == 2:
            return self._embed(img, encoded, k)
        elif img.ndim == 3:
            output = numpy.zeros(img.shape)
            for i in range(img.shape[2]):
                output[:,:,i] = self._embed(img[:,:,i], encoded, k)
            return output
        else:
            raise TypeError("img must be a 2d or 3d array")
    
    def _extract(self, img):
        cA, (cH, cV, cD) = dwt2(img, self.mother)
        arr = self._interleave(cH, cV, cD)
        chunk_size = arr.size // self.total_bits
        seq0 = self.seq0[:chunk_size]
        seq1 = self.seq1[:chunk_size]

        byte = 0
        output = bytearray()
        for i in range(self.total_bits):
            chunk = arr[i * chunk_size: i * chunk_size + seq0.size]
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
            out = self.rscodec.decode(payload)
        except ReedSolomonError:
            #print "!!", repr(payload)
            try:
                rpayload = bytearray(b ^ 255 for b in payload)
                out = self.rscodec.decode(rpayload)
            except ReedSolomonError:
                #print "!!", repr(rpayload)
                out = None
        return out
    
    def extract(self, img):
        if img.ndim == 2:
            return self._try_decode(self._extract(img))
        elif img.ndim == 3:
            for i in range(img.shape[2]):
                out = self._try_decode(self._extract(img[:,:,i]))
                if out is not None:
                    return out
            return self._try_decode(self._extract(mean(img, 2)))
        else:
            raise TypeError("img must be a 2d or 3d array")

def test_jpg(w, img):
    prev_i = 100
    for i in range(prev_i, 0, -5):
        print i,
        misc.toimage(img).save("out.jpg", quality = i)
        if w.extract(misc.imread("out.jpg")) is None:
            return prev_i
        prev_i = i
    return "unbound"

def test_filter(w, img, filterfunc, rng):
    prev_i = -1
    for i in rng:
        print i,
        misc.imsave("out.png", filterfunc(img, i))
        if w.extract(misc.imread("out.png")) is None:
            return prev_i
        prev_i = i
    return "unbound"

def test_recursive_filter(w, img, filterfunc, rng):
    prev_i = -1
    misc.imsave("out.png", out)
    for i in rng:
        print i, 
        misc.imsave("out.png", filterfunc(misc.imread("out.png")))
        if w.extract(misc.imread("out.png")) is None:
            return prev_i
        prev_i = i
    return "unbound"


def add_noise(img, k, min, max):
    mask = numpy.zeros(img.size)
    mask[:k*img.size] = 1
    numpy.random.shuffle(mask)
    mask = mask.reshape(img.shape)
    noise = numpy.random.randint(min, max, img.shape)
    return img + noise * mask

def add_blocks(img, noise_ratio, w, h):
    img2 = numpy.array(img)
    area = numpy.zeros(img.shape[:2])
    min_noise = area.size * noise_ratio
    while numpy.sum(area) < min_noise:
        y0=randint(0, img.shape[0])
        x0=randint(0, img.shape[1])
        y1=randint(y0, y0+w)
        x1=randint(x0, x0+h)
        img2[x0:x1,y0:y1,:] = 0
        img2[x0:x1,y0:y1,randint(0,2)] = 255
        area[x0:x1,y0:y1] = 1
    return img2

if __name__ == "__main__":
    from scipy.ndimage.filters import gaussian_filter, gaussian_laplace, uniform_filter, median_filter
    from skimage.filter import tv_denoise
    from random import randint

    #w = Watermarker(6, 3, 2394181, "bior3.3")
    out = misc.imread("pics/cat.jpg")
    #out = w.embed(misc.imread("pics/cat.jpg"), "123456", 7)
    out2 = numpy.array(misc.toimage(out).rotate(7, expand=True))
    #misc.imsave("orig.png", out)
    #misc.imsave("rot.png", out2)
    
    from skimage.filter import threshold_otsu, canny, sobel, prewitt
    from skimage.transform import probabilistic_hough
    from scipy.cluster.vq import kmeans
    import math
    mono = mean(out2, 2)
    #bw = mono > threshold_otsu(mono)
    mono = sobel(mono)
    bw = mono > threshold_otsu(mono)
    #mono = misc.imfilter(misc.imfilter(mono, "edge_enhance_more"), "edge_enhance_more")
    #mono = misc.imfilter(mono, "smooth")
    #mono = canny(mono)
    misc.imsave("otsu.png", bw)
    lines = probabilistic_hough(bw, line_length = 400, line_gap = 300)
    def norm(line):
        (x0, y0), (x1,y1) = line
        return math.sqrt((x0-x1)**2 + (y0-y1)**2)
    lines = sorted(lines, key = norm, reverse = True)
    angles = [math.atan2(y1-y0, x1-x0) * (180 / math.pi) for (x0, y0), (x1,y1) in lines]
    perpendicular = [a for a in angles for b in angles if 85 <= abs(a - b) <= 95]
    
    #print angles
    #print perpendicular
    
    clusters, _ = kmeans(numpy.array(perpendicular), 4)
    print clusters
    
    
    #misc.imsave("noise.png", add_noise(out, 1, -128, 128))
    #misc.imsave("noise.png", add_blocks(out, 0.9, 40, 40))
    #print w.extract(misc.imread("noise.png"))
    
    
    
    #print "JPG quality"
    #print test_jpg(w, out)
    #print "Gaussian"
    #print test_filter(w, out, gaussian_filter, [i/10.0 for i in range(1,50)])
    #print "LoG"
    #print test_filter(w, out, gaussian_laplace, [i/10.0 for i in range(1,50)])
    #for flt in ["blur", "contour", "detail", "edge_enhance", "edge_enhance_more", "emboss", 
    #        "find_edges", "smooth", "smooth_more", "sharpen"]:
    #    print flt
    #    print test_recursive_filter(w, out, lambda img: misc.imfilter(img, flt), range(1, 30))
    #print "TV denoise"
    #print test_filter(w, out, tv_denoise, range(50,1000,25))
    

