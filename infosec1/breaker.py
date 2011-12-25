addresses = [int(addr) for addr in open("indexes.txt", "r").read().splitlines()]
cipher = [int(num) for num in open("ciphertextOrd.txt", "r").read().splitlines()]
plain = [ord(ch) for ch in open("plaintext.txt", "r").read()]

unique_addresses = set(addresses)
assert len(unique_addresses) == 256 # pigeonhole principle
base = min(unique_addresses)
addresses_to_indexes = {addr : addr - base for addr in unique_addresses}
indexes = [addresses_to_indexes[addr] for addr in addresses]
assert len(indexes) % 6 == 0
assert len(indexes) / 6 == len(cipher)

info = []
for i in range(len(cipher)):
    block = indexes[i*6:i*6+6]
    assert block[0] == block[2] == block[3]
    assert block[1] == block[4]
    info.append((block[0], block[1], block[5], cipher[i], plain[i] if i < len(plain) else None))

secret = [None] * 256
yprev = 0  

for x, y, ind, cipher, plain in info:
    if plain is None:
        break
    secret[x] = (y - yprev) % 256
    yprev = y
    secret[x], secret[y] = secret[y], secret[x]
    secret[x] = (ind - secret[y]) % 256
    secret[ind] = plain ^ cipher

print secret

f = open("secret.txt", "w")
f.write("\n".join(str(num) for num in secret))
f.close()

"""
x_prev = 0
y_prev = 0
for plain in plaintext:
    x = (x_prev + 1) % 256
    x_prev = x
    y = (y_prev + secret[x]) % 256
    y_prev = y
    
    KNOW x
    
    >>> secret[x] = (y - y_prev) % 256
    
    SWAP secret[x], secret[y]  
    
    KNOW y
    
    >>> secret[x], secret[y] = secret[y], secret[x]
        
    ind = (secret[x] + secret[y]) % 256
    cipher = plain ^ secret[ind]
    
    KNOW ind
    
    >>> secret[x] = (secret[y] - ind) % 256
    
    >>> since we know both `cipher` and `plain` for the first 10000 bytes, we know that
    >>> secret[ind] = cipher ^ plain 
    
"""















