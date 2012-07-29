import os
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

def test_filter(w, img, fmt, filterfunc, rng):
    prev_i = None
    prev_fn = None
    for i in rng:
        fn = fmt % (i,)
        misc.imsave(fn, filterfunc(img, i))
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

def test_recursive_filter(w, img, fmt, filterfunc, rng):
    prev_i = None
    misc.imsave("tmp.png", img)
    for i in rng:
        misc.imsave("tmp.png", filterfunc(misc.imread("tmp.png")))
        try:
            w.extract(misc.imread("tmp.png"))
        except ReedSolomonError:
            os.rename("tmp.png", fmt % (prev_i,))
            return prev_i
        prev_i = i
    os.rename("tmp.png", fmt % (prev_i,))
    return "unbound"

def add_noise(img, k, min = -200, max = 200):
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
    os.chdir(os.path.dirname(os.path.abspath(__file__)))
    ROOT = os.getcwd()
    if not os.path.isdir("results"):
        os.mkdir("results")
    
    for fn in os.listdir("pics"):
        src = misc.imread(os.path.join(ROOT, "pics", fn))
        for k in k_range:
            dst = os.path.join(ROOT, "results", "%s-%d" % (fn.split(".")[0], k))
            try:
                os.mkdir(dst)
            except EnvironmentError:
                print "skipping", fn, k
                continue
            os.chdir(dst)
            out = w.embed(src, payload, k)
            misc.imsave("orig.png", out)
            
            print "%s (k = %d)" % (fn, k)
            print "=" * 20
            print "Minimum JPG quality", test_jpg(w, out)
            print "Max Gaussian sigma:", test_filter(w, out, "gauss-%s.png", gaussian_filter, 
                [i/10.0 for i in range(1,50)])
            print "Max Laplacian of Gaussian sigma:", test_filter(w, out, "log-%s.png", gaussian_laplace, 
                [i/10.0 for i in range(1,50)])
            print "Max tv-denoising weight:", test_filter(w, out, "tv-%s.png", tv_denoise, 
                range(50,1000,25))
            print "Max noise ratio: ", test_filter(w, out, "noise-%s.png", add_noise, 
                [i/20.0 for i in range(1, 21)]) 
            print "Max random block coverage: ", test_filter(w, out, "blocks-%s.png", 
                lambda img, k: add_blocks(img, k, 40, 40), [i/20.0 for i in range(1, 21)]) 

            for flt in ["contour", "detail", "edge_enhance", "edge_enhance_more", "emboss", 
                    "find_edges", "smooth", "smooth_more", "sharpen"]:
                print "Max iterations of %r: %s" % (flt, test_recursive_filter(w, out, 
                    "%s-%%s.png" % (flt,), lambda img: misc.imfilter(img, flt), range(1, 30)))
            print


if __name__ == "__main__":
    w = Watermarker(6, 4)
    run_tests(w, "123456", [2,4,6,8])



    
    

