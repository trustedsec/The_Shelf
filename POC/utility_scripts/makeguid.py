import sys

if len(sys.argv) < 2:
  print("usage: {} IIDString without")


IID = sys.argv[1].strip('{}')
p1, p2, p3, p4, p5 = IID.split('-')
parts = []
parts.append("{{0x{}".format(p1))
parts.append("0x{}".format(p2))
parts.append("0x{}".format(p3))
parts.append("{{0x{},0x{}".format(p4[:2], p4[2:]))
parts.append("0x{},0x{},0x{},0x{},0x{},0x{}}}}};".format(p5[0:2], p5[2:4], p5[4:6], p5[6:8], p5[8:10], p5[10:12]))
print(",".join(parts))
