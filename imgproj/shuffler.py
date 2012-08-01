import random
random.seed(23861)

b = [11*i for i in range(15)]
r = range(len(b))
random.shuffle(r)
c = [b[i] for i in r]
print c

d = [None] * len(b)
for i, x in enumerate(c):
    d[r[i]] = x
print d

