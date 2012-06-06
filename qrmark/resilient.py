import numpy
from scipy.fftpack import fft2, ifft
from scipy import misc
from skimage.filter import canny, sobel, threshold_otsu
from skimage.transform import probabilistic_hough
from PIL import Image

def distance(vec):
    dx, dy = vec
    return numpy.sqrt(dx*dx + dy*dy)

def line_to_vector(l):
    (x1, y1), (x2, y2) = l
    return x2 - x1, y2 - y1

def align(img, min_line_length = 100):
    sob = sobel(img)
    bw = sob > threshold_otsu(sob)
    lines = probabilistic_hough(bw, line_length = min_line_length)
    sorted_lines = sorted(lines, key = lambda l: distance(line_to_vector(l)), 
        reverse = True)[:10]

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
    return misc.imrotate(expanded, 7)


def embed_watermark(img, payload, block_size = 16):
    output = numpy.zeros(img.shape)
    for i in range(0, img.shape[0], block_size):
        for j in range(0, img.shape[1], block_size):
            block = img[i:i+block_size, j:j+block_size]
            if block.shape != (block_size, block_size):
                continue
            fftblock = fft2(block)
            info = sum(fftblock[u,v] for u in range())
            
            
            output[i:i+block_size, j:j+block_size] = ifft2(fftblock)
    return output





if __name__ == "__main__":
    orig = misc.lena()
    rot = expand_rotate(orig, 7)
    img2 = align(rot)
    print img2.shape
    misc.imsave("out.png", img2)
    

