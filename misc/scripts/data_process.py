import ntpath
import math

class delta:
    val_prev = 0

    def evaluate(self, val):
        delta  = val - self.val_prev
        self.val_prev = val 
        return delta

path = "C:/Work/prj/test_osg/data/log_e_su_posadka.txt"
inputfile = open(path)
outputfile = open(path+"_out.txt",'w')

delta_x = delta()
delta_y = delta()
delta_w = delta()
delta_t = delta()

out_str = ''
for line in inputfile:
    if len(line)==0 :
           continue

    out_str = line.split(' ')
    dx = delta_x.evaluate(float(out_str[0].split('=')[1]))
    dy = delta_y.evaluate(float(out_str[1].split('=')[1]))
    dw = delta_w.evaluate(float(out_str[5].split('=')[1]))
    dt = delta_t.evaluate(float(out_str[8].split('=')[1]))
    
    #print ( math.sqrt(dx*dx + dy*dy) )
          
    out_str = ("dx= %0.2f  dv = %0.2f acc = %0.2f") % (math.sqrt(dx*dx + dy*dy), dw, dw / dt ) + " " + line
    outputfile.write(out_str )
    

outputfile.close()
inputfile.close()
