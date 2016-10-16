execfile("D:/Work_terminal/Terminal/Options_pb2.py")

p = Ports()
p.GGSPort = 13000

if p.HasField('GGSPort'):
   print "  GGS port:", p.GGSPort