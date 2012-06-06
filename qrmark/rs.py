def hamming_weight(x):
   weight = 0
   while x > 0:
      weight += x & 1
      x >>= 1
   return weight

def poly_mult(x,y):
   z = 0
   i = 0
   while (y>>i) > 0:
      if y & (1<<i):
         z ^= x<<i
      i += 1
   return z

gf_exp = [1] * 512
gf_log = [0] * 256
x = 1
for i in range(1,255):
   x <<= 1
   if x & 0x100:
      x ^= 0x11d
   gf_exp[i] = x
   gf_log[x] = i
for i in range(255,512):
   gf_exp[i] = gf_exp[i-255]

def gf_mul(x,y):
   if x==0 or y==0:
      return 0
   return gf_exp[gf_log[x] + gf_log[y]]

def gf_div(x,y):
   if y==0:
      raise ZeroDivisionError()
   if x==0:
      return 0
   return gf_exp[gf_log[x] + 255 - gf_log[y]]

def gf_poly_scale(p,x):
   r = [0] * len(p)
   for i in range(0, len(p)):
      r[i] = gf_mul(p[i], x)
   return r

def gf_poly_add(p,q):
   r = [0] * max(len(p),len(q))
   for i in range(0,len(p)):
      r[i+len(r)-len(p)] = p[i]
   for i in range(0,len(q)):
      r[i+len(r)-len(q)] ^= q[i]
   return r

def gf_poly_mul(p,q):
   r = [0] * (len(p)+len(q)-1)
   for j in range(0, len(q)):
      for i in range(0, len(p)):
         r[i+j] ^= gf_mul(p[i], q[j])
   return r

def gf_poly_eval(p,x):
   y = p[0]
   for i in range(1, len(p)):
      y = gf_mul(y,x) ^ p[i]
   return y

def rs_generator_poly(nsym):
   g = [1]
   for i in range(0,nsym):
      g = gf_poly_mul(g, [1, gf_exp[i]])
   return g

def rs_encode_msg(msg_in, nsym):
   gen = rs_generator_poly(nsym)
   msg_out = [0] * (len(msg_in) + nsym)
   for i in range(0, len(msg_in)):
      msg_out[i] = msg_in[i]
   for i in range(0, len(msg_in)):
      coef = msg_out[i]
      if coef != 0:
         for j in range(0, len(gen)):
            msg_out[i+j] ^= gf_mul(gen[j], coef)
   for i in range(0, len(msg_in)):
      msg_out[i] = msg_in[i]
   return msg_out

def rs_calc_syndromes(msg, nsym):
   synd = [0] * nsym
   for i in range(0, nsym):
      synd[i] = gf_poly_eval(msg, gf_exp[i])
   return synd

def rs_correct_errata(msg, synd, pos):
   # calculate error locator polynomial
   q = [1]
   for i in range(0, len(pos)):
      x = gf_exp[len(msg)-1-pos[i]]
      q = gf_poly_mul(q, [x,1])
   # calculate error evaluator polynomial
   p = synd[0:len(pos)]
   p.reverse()
   p = gf_poly_mul(p, q)
   p = p[len(p)-len(pos):len(p)]
   # formal derivative of error locator eliminates even terms
   q = q[len(q)&1:len(q):2]
   # compute corrections
   for i in range(0, len(pos)):
      x = gf_exp[pos[i]+256-len(msg)]
      y = gf_poly_eval(p, x)
      z = gf_poly_eval(q, gf_mul(x,x))
      msg[pos[i]] ^= gf_div(y, gf_mul(x,z))

def rs_find_errors(synd, nmess):
   # find error locator polynomial with Berlekamp-Massey algorithm
   err_poly = [1]
   old_poly = [1]
   for i in range(0,len(synd)):
      old_poly.append(0)
      delta = synd[i]
      for j in range(1,len(err_poly)):
         delta ^= gf_mul(err_poly[len(err_poly)-1-j], synd[i-j])
      if delta != 0:
         if len(old_poly) > len(err_poly):
            new_poly = gf_poly_scale(old_poly, delta)
            old_poly = gf_poly_scale(err_poly, gf_div(1,delta))
            err_poly = new_poly
         err_poly = gf_poly_add(err_poly, gf_poly_scale(old_poly, delta))
   errs = len(err_poly)-1
   if errs*2 > len(synd):
      return None    # too many errors to correct
   # find zeros of error polynomial
   err_pos = []
   for i in range(0, nmess):
      if gf_poly_eval(err_poly, gf_exp[255-i]) == 0:
         err_pos.append(nmess-1-i)
   if len(err_pos) != errs:
      return None    # couldn't find error locations
   return err_pos

def rs_forney_syndromes(synd, pos, nmess):
   fsynd = list(synd)      # make a copy
   for i in range(0, len(pos)):
      x = gf_exp[nmess-1-pos[i]]
      for i in range(0,len(fsynd)-1):
         fsynd[i] = gf_mul(fsynd[i], x) ^ fsynd[i+1]
      fsynd.pop()
   return fsynd

def rs_correct_msg(msg_in, nsym):
   msg_out = list(msg_in)     # copy of message
   # find erasures
   erase_pos = []
   for i in range(0, len(msg_out)):
      if msg_out[i] < 0:
         msg_out[i] = 0
         erase_pos.append(i)
   if len(erase_pos) > nsym:
      return -1     # too many erasures to correct
   synd = rs_calc_syndromes(msg_out, nsym)
   if max(synd) == 0:
      return msg_out  # no errors
   fsynd = rs_forney_syndromes(synd, erase_pos, len(msg_out))
   err_pos = rs_find_errors(fsynd, len(msg_out))
   if err_pos is None:
      return -2    # error location failed
   rs_correct_errata(msg_out, synd, erase_pos + err_pos)
   synd = rs_calc_syndromes(msg_out, nsym)
   if max(synd) > 0:
      return -3     # message is still not right
   return msg_out


def rs_encode(msg):
    msg += "\x00"
    enc = rs_encode_msg([ord(ch) for ch in msg + "\x00"], 10)
    return "".join(chr(x) for x in enc)

def rs_decode(data):
    dec = rs_correct_msg([ord(ch) for ch in data], 10)
    if dec == -2:
        return data.split("\x00")[0]
    elif dec < 0:
        raise ValueError("too many errors in input")
    return "".join(chr(x) for x in dec).split("\x00")[0]

print rs_decode("X" + rs_encode("hello world")[1:])







