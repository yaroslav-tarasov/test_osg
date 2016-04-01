import ntpath

path = "C:/Work/prj/test_osg/data/log_e_ka50_.txt"
inputfile = open(path)
outputfile = open(path+"_out.txt",'w')

out_str = ''
for line in inputfile:
    if len(line)==0 :
           continue

    if out_str =='' :
            out_str = line
            out_str = ''.join(out_str.split('\n'))
            continue
    out_str = out_str + line
    outputfile.write(out_str )
    out_str= ''

outputfile.close()
inputfile.close()
