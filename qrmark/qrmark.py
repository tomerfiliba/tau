import qrcode
from scipy import misc
from PIL import Image
import numpy

def generate_qr(text):
    qr = qrcode.QRCode(error_correction = qrcode.constants.ERROR_CORRECT_H,
        box_size = 10, border = 4)
    qr.add_data(text)
    qr.make(fit = True)
    img = qr.make_image()._img
    return numpy.array(img).reshape(img.size[0], img.size[1])

host = misc.lena()
payload = generate_qr("Hello world")

def embed_lsb(host, payload):
    for r in range(len(host)):
        if r >= len(payload):
            break
        hrow = host[r]
        prow = host[r]
        for c in range(len(hrow)):
            if c >= len(prow):
                break
            if prow[c]:
                hrow[c] |= 1
            else:
                hrow[c] &= 0xFE
    return host

embedded = embed_lsb(host, payload)
misc.imsave('emb.png', embedded)

#def decode_lsb(host):
#    for row in host:
#        pass







