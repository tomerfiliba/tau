class Matrix(object):
    def __init__(self, width, height):
        self.elements = [[None] * width for i in range(height)]
    def __getitem__(self, (x, y)):
        return self.elements[y][x]
    def __setitem__(self, (x, y), value):
        self.elements[y][x] = value
    def show(self):
        for row in self.elements:
            line = ""
            for elem in row:
                if elem is None:
                    line += " -"
                else:
                    line += " %s" % (elem,)
            print line
            #for x in len(self.elements[y]):
            #    print 


def fill_matrix(src, dst, ins_cost, del_cost, subst_cost):
    src = "$" + src
    dst = "$" + dst
    m = Matrix(len(src)+1, len(dst)+1)
    for i, ch in enumerate(src):
        m[0, i+1] = ch
        m[1, i+1] = i
    for i, ch in enumerate(dst):
        m[i+1, 0] = ch
        m[i+1, 1] = i
    return m


m = fill_matrix("kitten", "kettle", lambda x: 1, lambda x: 1, lambda x, y: 2)
m.show()
print "distance:", m[-1, -1]









