import numpy
import math
from pylab import mean
from pywt import dwt2, idwt2
from scipy.fftpack import fft2, ifft
from scipy.stats import pearsonr
from scipy import misc
from skimage.filter import canny, sobel, threshold_otsu
from skimage.transform import probabilistic_hough
from PIL import Image
from random import Random
from reedsolo import RSCodec


def distance(vec):
    dx, dy = vec
    return numpy.sqrt(dx*dx + dy*dy)

def line_to_vector(l):
    (x1, y1), (x2, y2) = l
    return x2 - x1, y2 - y1

def align(img, min_line_length = 100):
    sob = sobel(img)
    print "!! sob"
    bw = sob > threshold_otsu(sob)
    print "!! otsu"
    lines = probabilistic_hough(bw, line_length = min_line_length)
    print "!! hough"
    sorted_lines = sorted(lines, key = lambda l: distance(line_to_vector(l)), 
        reverse = True)[:10]
    
    print "!!", sorted_lines

    rotations = {}
    for l1 in sorted_lines:
        v1 = line_to_vector(l1)
        for l2 in sorted_lines:
            if l1 == l2:
                continue
            v2 = line_to_vector(l2)
            theta = numpy.arccos(numpy.dot(v1, v2) / (distance(v1) * distance(v2)))
            if abs(numpy.degrees(theta) - 90) <= 1:
                # found an alignment!
                angle = int(round(numpy.degrees(numpy.arccos(numpy.dot(v1, (0, 1)) / distance(v1)))))
                if angle > 90:
                    angle = -(angle % 90)
                if angle > 45:
                    angle = 90 - angle
                elif angle < -45:
                    angle = -90 - angle
                if angle not in rotations:
                    rotations[angle] = 0
                rotations[angle] += 1
    
    if not rotations:
        # couldn't find boundaries, assume aligned 
        return img
    
    angle = max(rotations.items(), key = lambda item: item[1])[0]
    img2 = misc.imrotate(img, angle)
    
    sob = sobel(img2)
    bw = sob > threshold_otsu(sob)
    lines = probabilistic_hough(bw, line_length = min_line_length)
    sorted_lines = sorted(lines, key = lambda l: distance(line_to_vector(l)), 
        reverse = True)[:4]
    
    min_y = bw.shape[0]
    max_y = 0
    min_x = bw.shape[1]
    max_x = 0
    
    for l in sorted_lines:
        (x1, y1), (x2, y2) = l
        if x1 < min_x:
            min_x = x1
        if x1 > max_x:
            max_x = x1
        if x2 < min_x:
            min_x = x2
        if x2 > max_x:
            max_x = x2
        if y1 < min_y:
            min_y = y1
        if y1 > max_y:
            max_y = y1
        if y2 < min_y:
            min_y = y2
        if y2 > max_y:
            max_y = y2
    
    img3 = img2[min_y+1:max_y, min_x+1:max_x]
    #return misc.imresize(img3, (512, 512))
    return img3

def expand_rotate(img, angle):
    expanded = numpy.zeros((img.shape[0] * 2, img.shape[1] * 2), img.dtype)
    expanded[img.shape[0] / 2:img.shape[0] / 2 + img.shape[0], img.shape[1] / 2: img.shape[1] / 2 + img.shape[1]] = img
    return misc.imrotate(expanded, angle)


def iterbits(data):  # MSB first
    for n in data:
        for i in (7,6,5,4,3,2,1,0):
            yield (n >> i) & 1


class Watermarker(object):
    def __init__(self, msg_bytes, ec_bytes, seed):
        self.seed = seed
        self.rscodec = RSCodec(ec_bytes)
        self.msg_bytes = msg_bytes
        self.total_bits = (msg_bytes + ec_bytes) * 8
        self.rand = Random()
    def _reset(self):
        self.rand.seed(self.seed)
    
    def embed(self, img, payload, k = 2):
        img = grayscale(img)
        if len(payload) > self.msg_bytes:
            raise ValueError("payload too long")
        self._reset()
        cA, (cH, cV, cD) = dwt2(img, "haar")
        
        w, h = cH.shape
        cH2 = cH.reshape(cH.size)
        cV2 = cV.reshape(cV.size)
        chunk_size = cH2.size // (self.total_bits // 2)
        
        encoded = self.rscodec.encode(payload)
        for i, bit in enumerate(iterbits(encoded)):
            seq = [self.rand.choice([k, 0]) for _ in range(chunk_size)]
            dst = (cH2, cV2)[i % 2]
            if bit:
                dst[(i//2)*chunk_size:(i//2+1)*chunk_size] += seq
        
        return idwt2((cA, (cH2.reshape(w, h), cV2.reshape(w, h), cD)), "haar")

    def extract(self, img):
        img = grayscale(img)
        self._reset()
        cA, (cH, cV, cD) = dwt2(img, "haar")

        w, h = cH.shape
        cH2 = cH.reshape(cH.size)
        cV2 = cV.reshape(cV.size)
        chunk_size = cH2.size // (self.total_bits // 2)
        
        bytes = ""
        byte = 0
        for i in range(self.total_bits):
            seq = [self.rand.choice([1, 0]) for _ in range(chunk_size)]
            src = (cH2, cV2)[i % 2]
            chunk = src[(i//2)*chunk_size:(i//2+1)*chunk_size]
            corr, _ = pearsonr(chunk, seq)
            bit = int(corr > 0.3)
            byte = bit | (byte << 1)
            if i % 8 == 7:
                bytes += chr(byte)
                byte = 0
        print repr(bytes)
        return self.rscodec.decode([ord(b) for b in bytes])

def grayscale(img):
    if len(img.shape) == 2:
        return img
    else:
        return mean(img, 2)


if __name__ == "__main__":
    orig = misc.lena()
    w = Watermarker(8, 4, 239047238847)
    #img2 = w.embed(orig, "helloman", 20)
    #misc.imsave("out.png", img2)

    #img3 = misc.imread("degraded60.jpg")
    #payload = w.extract(mean(img3, 2))
    src = misc.imread("lomo1.png")
    payload = w.extract(src)
    print repr(payload)
    
    #print "out"
    #img3 = misc.imread("out.jpg")
    #img3 = misc.imread("scan2.png")
    #payload = w.extract(img4)
    #print repr(payload)
    #src = misc.imread("scan2.png")
    #img2 = align(src)
    #misc.imsave("out2.png", img2)
    #payload = w.extract(img2)
    #print repr(payload)
    
    #rot = expand_rotate(orig, 7)
    #img2 = align(rot)
    #print img2.shape
    #misc.imsave("out.png", img2)
    

