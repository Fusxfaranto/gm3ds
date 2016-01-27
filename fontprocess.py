from copy import deepcopy

l = [[0 for x in xrange(5)] for x in xrange(6)]
a = [deepcopy(l) for x in xrange(59)]

with open('font.data', 'rb') as f:
    for row in xrange(6):
        for i in xrange(59):
            for col in xrange(5):
                a[i][row][col] = int(not ord(f.read(1)))
            if i != 58:
                f.read(1)
for let in a:
    print str(let).replace('[', '{').replace(']', '}') + ','
