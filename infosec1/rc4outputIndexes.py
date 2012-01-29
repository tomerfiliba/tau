#!/usr/bin/env python
#
#       RC4, ARC4, ARCFOUR algorithm
#
#       Copyright (c) 2009 joonis new media
#       Author: Thimo Kraemer <thimo.kraemer@joonis.de>
#       Modified by Roei Schuster for Tel Aviv University course
#       Information Security: Theory vs. Reality, 2011-12.
# 
#       This program is free software; you can redistribute it and/or modify
#       it under the terms of the GNU General Public License as published by
#       the Free Software Foundation; either version 2 of the License, or
#       (at your option) any later version.
#       
#       This program is distributed in the hope that it will be useful,
#       but WITHOUT ANY WARRANTY; without even the implied warranty of
#       MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#       GNU General Public License for more details.
#       
#       You should have received a copy of the GNU General Public License
#       along with this program; if not, write to the Free Software
#       Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
#       MA 02110-1301, USA.
#

def rc4crypt(data, key, offset):
    x = 0
    secret = range(256)
    f = open(r'indexes.txt','w')
    for i in range(256):
        x = (x + secret[i] + ord(key[i % len(key)])) % 256
        secret[i], secret[x] = secret[x], secret[i]
    fS = open(r'C:\temp\S.txt','w')
    for i in range(256):
        fS.write(hex(secret[i]) + '\n')
    fS.close()
    x = 0
    y = 0
    out = []
    for char in data:
        x = (x + 1) % 256
        y = (y + secret[x]) % 256
        f.write((str(x + offset))[2:] + '\n') #read access
		
        secret[x], secret[y] = secret[y], secret[x]
        f.write((str(y + offset))[2:] + '\n') # first read access of swap
        f.write((str(x + offset))[2:] + '\n') # second read access of swap
		
		# Now, we XOR the generated pseudorandom stream byte with the plaintext/ciphertext byte.
        out.append(chr(ord(char) ^ secret[(secret[x] + secret[y]) % 256]))
		# Read accesses:
        f.write((str(x + offset))[2:] + '\n')
        f.write((str(y + offset))[2:] + '\n')
        f.write((str((secret[x] + secret[y])%256 + offset))[2:] + '\n')
    f.close()
    return ''.join(out)
