import qrcode
from scipy.fftpack import dct
from scipy import misc
#from PIL import Image
import numpy

def generate_qr(text):
    qr = qrcode.QRCode(error_correction = qrcode.constants.ERROR_CORRECT_H,
        box_size = 1)
    qr.add_data(text)
    qr.make(fit = True)
    img = qr.make_image()._img
    return numpy.array(img.getdata()).reshape(img.size[0], img.size[1])

#host = misc.lena()
payload = generate_qr("Hello world")



#misc.imsave("pay.png", payload)

#for row in payload:
#    for col in row:
#        print 1 - col / 255,
#    print



#def embed(host, payload):
#    assert host.shape >= payload.shape
#    for i in range(payload.shape[0]):
#        for j in range(payload.shape[1]):
#            if payload[i][j]:
#                host[i][j] |= 1
#            else:
#                host[i][j] &= 0xFE
#    return host
#
#emb = embed(host, payload)
#misc.imsave("emb.png", emb)
#
#dec = (emb & 1) * 255
#misc.imsave("dec.png", dec)
#
#misc.imsave("emb.jpg", emb)
#emb2 = misc.imread("emb.jpg")
#dec2 = (emb2 & 1) * 255
#misc.imsave("dec2.png", dec2)



