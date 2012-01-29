#!/usr/bin/env python
#
#       RC4, ARC4, ARCFOUR algorithm
#
#		Author: Roei Schuster, Eran Tromer
#		Created for Tel Aviv University course Information
#		Security: Theory vs. Reality, 2011-12
#       RC4 implementation based on one by Thimo Kraemer <thimo.kraemer@joonis.de>,
#		joonis new media, 2009
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

def removeEmptyLast(lineList):
	if (lineList[-1] == ''): # -1 denotes the greates index of the list
		return lineList[:-1]
	else:
		return lineList

def getFileContentAsLineList(fileName):
	f = open(fileName, 'r')
	content = f.read().split('\n') # f as a list of lines
	f.close()
	return removeEmptyLast(content) # ignore tailing empty lines
		
def rc4crypt(data, secret, iteration, jValue):
	'''
	Encrypt or decrypt data after the offset
	Copyright (c) 2009 joonis new media
	Author: Thimo Kraemer <thimo.kraemer@joonis.de>
	Full rc4 python implementation at: http://code.activestate.com/recipes/576736-rc4-arc4-arcfour-algorithm/
	'''
	i = iteration % 256 # The value of i after 10,000 iterations
	j = jValue # The value of j after 10,000 iterations
	out = []
	for char in data:
		i = (i + 1) % 256
		j = (j + secret[i]) % 256
		secret[i], secret[j] = secret[j], secret[i]
		out.append(chr(ord(char) ^ secret[(secret[i] + secret[j]) % 256]))
	return ''.join(out)

def openCiphertext(ciphertextFileName):
	'''
	Opens ciphertext file. Ciphertext is returned as a list of characters.
	'''
	data = getFileContentAsLineList(ciphertextFileName)
	data = [chr(int(x)) for x in data]
	return data

def openSecret(secretFileName):
	'''
	Opens file with secret stored in it. Secret values are returned as integers.
	'''
	secret = getFileContentAsLineList(secretFileName)
	if (len(secret) != 256):
		print "ERROR: table length wrong."
		return
	secret = [int(x) for x in secret]
	return secret
	
	
data = openCiphertext('ciphertextOrd.txt')[10000:]
secret = openSecret('secret.txt')
print rc4crypt(data, secret, 10000, 54)

