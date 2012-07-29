import os
import shutil
import numpy
from watermarker import Watermarker
from scipy import misc
from scipy.ndimage.filters import gaussian_filter, gaussian_laplace, uniform_filter, median_filter
from skimage.filter import tv_denoise
from random import randint
from reedsolo import ReedSolomonError


def test_jpg(w, img):
    prev_i = None
    prev_fn = None
    for i in range(100, 0, -5):
        fn = "jpg-%d.jpg" % (i,)
        misc.toimage(img).save(fn, "JPEG", quality = i)
        try:
            w.extract(misc.imread(fn))
        except ReedSolomonError:
            os.remove(fn)
            return prev_i
        if prev_fn:
            os.remove(prev_fn)
        prev_i = i
        prev_fn = fn
    return "unbound"

def test_filter(w, img, filterfunc, rng):
    prev_i = None
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

def run_tests(w, payload, k_range):
    os.chdir(os.path.dirname(__file__))
    ROOT = os.getcwd()
    if os.path.isdir("results"):
        shutil.rmtree("results")
    os.mkdir("results")
    
    for fn in os.listdir("pics"):
        src = misc.imread(os.path.join(ROOT, "pics", fn))
        for k in k_range:
            out = w.embed(src, payload, k)
            dst = os.path.join(ROOT, "results", "%s-%d" % (fn.split(".")[0], k))
            os.mkdir(dst)
            os.chdir(dst)
            misc.imsave("orig.png", out)
            
            print "%s (k = %d)" % (fn, k)
            print "=" * 20
            print "Minimum JPG quality", test_jpg(w, out)
            print "Max Gaussian sigma", test_filter(w, out, "gauss-%d.png", gaussian_filter, 
                [i/10.0 for i in range(1,50)])
            #print 
            #print "LoG"
            #print test_filter(w, out, gaussian_laplace, [i/10.0 for i in range(1,50)])
            #for flt in ["blur", "contour", "detail", "edge_enhance", "edge_enhance_more", "emboss", 
            #        "find_edges", "smooth", "smooth_more", "sharpen"]:
            #    print flt
            #    print test_recursive_filter(w, out, lambda img: misc.imfilter(img, flt), range(1, 30))
            #print "TV denoise"
            #print test_filter(w, out, tv_denoise, range(50,1000,25))
            #misc.imsave("noise.png", add_noise(out, 1, -128, 128))
            #misc.imsave("noise.png", add_blocks(out, 0.9, 40, 40))
            #print w.extract(misc.imread("noise.png"))
            print 


if __name__ == "__main__":
    w = Watermarker(6, 3)
    run_tests(w, "123456", [2,4,6,8])



    
    

