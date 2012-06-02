import math
import numpy
from scipy.fftpack import dct, idct
from scipy import misc

def encode(data):
    output = [None, None, None]
    for ch in data:
        bits = bin(ord(ch))[2:]
        bits = [int(b) for b in "0" * (8 - len(bits)) + bits]
        for b in bits:
            output.extend([b] * 3)
    output.extend([None, None, None])
    return output

middle_freq = numpy.array([
    [0, 0, 0, 1, 1, 1, 1, 0], 
    [0, 0, 1, 1, 1, 1, 0, 0], 
    [0, 1, 1, 1, 1, 0, 0, 0], 
    [1, 1, 1, 1, 0, 0, 0, 0], 
    [1, 1, 1, 0, 0, 0, 0, 0], 
    [1, 1, 0, 0, 0, 0, 0, 0], 
    [1, 0, 0, 0, 0, 0, 0, 0], 
    [0, 0, 0, 0, 0, 0, 0, 0], 
])

one_seq = numpy.array([
    [ 0, 0, 0, 1, 1,-1, 1, 0], 
    [ 0, 0,-1, 1, 1, 1, 0, 0], 
    [ 0, 1,-1,-1,-1, 0, 0, 0], 
    [-1, 1,-1,-1, 0, 0, 0, 0], 
    [ 1, 1,-1, 0, 0, 0, 0, 0], 
    [ 1, 1, 0, 0, 0, 0, 0, 0], 
    [ 1, 0, 0, 0, 0, 0, 0, 0], 
    [ 0, 0, 0, 0, 0, 0, 0, 0]
], float)

zero_seq = numpy.array([
    [ 0, 0, 0,-1,-1, 1,-1, 0],
    [ 0, 0, 1,-1,-1,-1, 0, 0],
    [ 0,-1, 1,-1, 1, 0, 0, 0],
    [-1,-1, 1, 1, 0, 0, 0, 0],
    [ 1,-1, 1, 0, 0, 0, 0, 0],
    [-1, 1, 0, 0, 0, 0, 0, 0],
    [-1, 0, 0, 0, 0, 0, 0, 0],
    [ 0, 0, 0, 0, 0, 0, 0, 0],
], float)

empty_seq = numpy.array([
    [ 0, 0, 0,-1, 1,-1,-1, 0],
    [ 0, 0,-1, 1, 1, 1, 0, 0],
    [ 0, 1, 1, 1,-1, 0, 0, 0],
    [ 1, 1, 1, 1, 0, 0, 0, 0],
    [ 1,-1,-1, 0, 0, 0, 0, 0],
    [ 1,-1, 0, 0, 0, 0, 0, 0],
    [-1, 0, 0, 0, 0, 0, 0, 0],
    [ 0, 0, 0, 0, 0, 0, 0, 0],
], float)


def embed_seq(dct_block, seq, k):
    for x in range(8):
        for y in range(8):
            if middle_freq[x, y]:
                dct_block[x, y] = dct_block[x, y] + (k * seq[x, y])

def embed(host, payload, k = 2):
    capacity = (host.shape[0] // 8) * (host.shape[1] // 8)
    assert len(payload) <= capacity

    watermarked = numpy.zeros(host.shape)
    p = 0
    for i in range(0, host.shape[0], 8):
        for j in range(0, host.shape[1], 8):
            block = host[i:i+8, j:j+8].astype(float)
            if block.shape != (8, 8):
                idct_block = block
            else:
                dct_block = dct(block, norm='ortho', overwrite_x = True)
                bit = payload[i % len(payload)]
                p += 1
                if bit == 0:
                    embed_seq(dct_block, zero_seq, k)
                elif bit == 1:
                    embed_seq(dct_block, one_seq, k)
                else:
                    embed_seq(dct_block, empty_seq, k)
                idct_block = idct(dct_block, norm='ortho', overwrite_x = True).astype(int)
            watermarked[i:i+8, j:j+8] = idct_block
    return watermarked


host = misc.lena()
payload = encode("hello")
watermarked = embed(host, payload, 2)
misc.imsave("out.png", watermarked)

def corr2(A, B):
    # http://www.mathworks.com/help/toolbox/images/ref/corr2.html
    meanA = numpy.mean(A)
    meanB = numpy.mean(A)
    x = sum((A[i,j] - meanA) * (B[i,j] - meanB) for i in range(A.shape[0]) for j in range(A.shape[1]))
    y = sum((A[i,j] - meanA) ** 2 for i in range(A.shape[0]) for j in range(A.shape[1]))
    z = sum((B[i,j] - meanB) ** 2 for i in range(A.shape[0]) for j in range(A.shape[1]))
    return x / math.sqrt(y * z)

def extract(watermarked):
    payload = []
    for i in range(0, watermarked.shape[0], 8):
        for j in range(0, watermarked.shape[1], 8):
            block = watermarked[i:i+8, j:j+8].astype(float)
            if block.shape != (8,8):
                continue
            dct_block = dct(block, norm='ortho', overwrite_x = True)
            dct_block *= middle_freq
            s0 = corr2(dct_block, zero_seq)
            s1 = corr2(dct_block, one_seq)
            se = corr2(dct_block, empty_seq)
            payload.append(max((s0, 0), (s1, 1), (se, None), key = lambda t: t[0])[1])
    return payload

payload2 = extract(watermarked)

def majority(seq):
    outputs = [[]]
    for i in range(0, len(seq), 3):
        chunk = seq[i:i + 3]
        if chunk.count(0) >= 2:
            outputs[-1].append(0)
        elif chunk.count(1) >= 2:
            outputs[-1].append(1)
        else:
            outputs.append([])
    return outputs

for seq in majority(payload2):
    text = ""
    for i in range(0, len(seq), 8):
        chunk = seq[i:i+8]
        text += chr(int("".join(str(n) for n in chunk), 2))
    if text:
        print repr(text)











