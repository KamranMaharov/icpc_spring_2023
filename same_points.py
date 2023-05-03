import sys
import re
f1 = sys.argv[1]
f2 = sys.argv[2]

lines1 = open(f1, 'r').readlines()
lines2 = open(f2, 'r').readlines()
assert(len(lines1) == len(lines2))

matcher = re.compile(r"(\d+\.?\d*) points")

different = False
for idx in range(len(lines1)):
    p1 = 0
    try:
        p1 = float(matcher.search(lines1[idx]).group(1))
    except:
        pass

    p2 = 0
    try:
        p2 = float(matcher.search(lines2[idx]).group(1))
    except:
        pass

    if p1 < p2:
        print("test " + str(idx + 1) + " : " + str(p1) + " < " + str(p2) )
        different = True
    elif p1 > p2:
        print("test " + str(idx + 1) + " : " + str(p1) + " > " + str(p2) )
        different = True

if not different:
    print("SAME")
