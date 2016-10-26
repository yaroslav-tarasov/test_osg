import ntpath
import re

path = "C:/Work/prj/test_osg/data/log_e_su_posadka_wo_ac.txt"
inputfile = open(path)
outputfile = open(path+"_out.txt",'w')

out_str = ''
for line in inputfile:
    if len(line)==0 :
           continue

    #if out_str =='' :
    #        out_str = line
    #        out_str = ''.join(out_str.split('\n'))
    #        continue
    #out_str = out_str + line
    
    #str = re.findall(r"[\w']+", line)
    str = re.split(r'[ =\n]+', line)
    time  = float(str[17])
    #str = line.split('=')
    # outputfile.write( ''.join(re.split(r'[\n]+', line)) + 'air_state=' + ('1' if time>72.3 else '5') + '\n')
    outputfile.write( ''.join(re.split(r'[\n]+', line)) + 'air_state=' + ('4' if time<250.0 else '5') + '\n')

    #out_str= ''

outputfile.close()
inputfile.close()
