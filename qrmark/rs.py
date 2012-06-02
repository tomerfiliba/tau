import itertools
import operator

mask = 'd3a0fe0d327cc6fa435f25edb36b9cc69960ec44'.decode("hex")

def encode(data, chunk_size = 4):
    output = []
    itermask = itertools.cycle(mask)
    for i in range(0, len(data), chunk_size):
        chunk = [ord(ch) ^ itermask.next() for ch in data[i:i+chunk_size]]
        xorsum = reduce(operator.xor, chunk, 0)
        chunk.append(xorsum)
        output.extend(chr(x) for x in chunk)
    return "".join(output)

def decode(data, chunk_size = 4):
    output = []
    itermask = itertools.cycle(mask)
    for i in range(0, len(data), chunk_size + 1):
        chunk = [ord(ch) ^ itermask.next() for ch in data[i:i+chunk_size]]
        xorsum = ord(data[i+chunk_size])
        output.extend(chr(x) for x in chunk)
    return "".join(output)

x = encode("hello")
print repr(x)
y = decode(x)
print repr(y)


