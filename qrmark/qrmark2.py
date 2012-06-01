import qrcode
from scipy.fftpack import dct, idct
from scipy import misc
import numpy

def generate_qr(text):
    qr = qrcode.QRCode(error_correction = qrcode.constants.ERROR_CORRECT_H,
        box_size = 1)
    qr.add_data(text)
    qr.make(fit = True)
    img = qr.make_image()._img
    return numpy.array(img.getdata()).reshape(img.size[0], img.size[1]) // 255

payload = generate_qr("Hello world")

host = misc.lena()

# capacity = host.shape[0] * host.shape[1] // 64

A = (0, 5)
B = (1, 4)
K = 4

def embed(host, payload, k):
    output = numpy.zeros(host.shape)
    for i in range(0, host.shape[0], 8):
        for j in range(0, host.shape[1], 8):
            block = host[i:i+8, j:j+8].astype(float)
            dct_block = dct(block, norm='ortho', overwrite_x = True)
            if i // 8 >= payload.shape[0] or j // 8 >= payload.shape[1]:
                bit = 1
            else:
                bit = payload[i // 8, j // 8]
            if bit:
                dct_block[A] = dct_block[B] + k + 1
                #if dct_block[A] < dct_block[B] - k:
            else:
                dct_block[A] = dct_block[B]
                #if dct_block[A] >= dct_block[B] - k:
                #    dct_block[A] = dct_block[B] - k - 1
            idct_block = idct(dct_block, norm='ortho', overwrite_x = True).astype(int)
            output[i:i+8, j:j+8] = idct_block
    return output


output = embed(host, payload, K)
misc.imsave("out.png", output)
misc.imsave("out.jpg", output)


def decode(watermarked, k):
    output = numpy.zeros((watermarked.shape[0] // 8, watermarked.shape[1] // 8))
    for i in range(0, watermarked.shape[0], 8):
        for j in range(0, watermarked.shape[1], 8):
            block = watermarked[i:i+8, j:j+8].astype(float)
            dct_block = dct(block, norm='ortho', overwrite_x = True)
            bit = dct_block[A] - dct_block[B] >= k
            output[i // 8, j // 8] = bit
    return output


watermarked = misc.imread("out.jpg")
pay2 = decode(watermarked, K)
misc.imsave("pay2.png", pay2)








