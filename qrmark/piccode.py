from reedsolo import RSCodec
from numpy import ndarray, zeros
from scipy.fftpack import dct, idct
from scipy.misc import imsave, imread


rs = RSCodec(40)

def encode(text):
    enc = rs.encode(text)
    dctarr = ()
    arr = zeros(32 * 32, float)
    for i, ch in enumerate(enc):
        bits = bin(ord(ch))[2:]
        bits = "0" * (8 - len(bits)) + bits
        for j, b in enumerate(bits):
            arr[i * 8 + j] = 255 if b == "1" else 0
    arr2 = arr.reshape((32, 32))
    return idct(arr2)

def decode(arr):
    arr2 = dct(arr.astype(float))
    arr3 = arr2.reshape(32 * 32)
    text = []
    for i in range(0, 60 * 8, 8):
        bits = "".join("0" if b < 128 else "1" for b in arr3[i:i+8])
        text.append(int(bits, 2))
    print repr(text)
    return rs.decode(text)



if __name__ == "__main__":
#    imsave("crap.jpg", encode("hello world"))
#    arr = imread("crap.jpg")
#    print repr(decode(arr))
    print idct(zeros((8,8), float))
    print idct(zeros((8,8), float) + 255)







