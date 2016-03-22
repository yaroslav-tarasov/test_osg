import ntpath

def isnumeric(s):
    '''Returns True for all non-unicode numbers'''
    try:
        s = s.decode('utf-8')
    except:
        return False

    try:
        float(s)
        return True
    except:
        return False
    
path = "C:/Work/models/external/aircrafts/l-39/l39/Models/l39.obj.mtl"
inputfile = open(path)
outputfile = open(ntpath.basename(path)+".txt",'w')
 

for line in inputfile:
    if len(line)==0 :
           outputfile.write(line )
           continue
    line = line.replace('\\','/')
    tokens = line.split()
    tokens = list(filter(lambda x: x!= '', tokens))
    if len(tokens)<2 or line[:1]=="#" :
            outputfile.write(line)
            continue

    if tokens[0]=="newmtl":
            outputfile.write(line)
            continue
        
    token0 = tokens[0].title() if tokens[0][0] in 'TKNtkn' else  tokens[0] 

    mapping = [ ('map_kd', 'map_Kd') ]
    for k, v in mapping:
        token0 = token0.replace(k, v)

    if tokens[0].lower()=="map_kd" :
            outputfile.write( '\t' + token0 + ' ' + tokens[1] + '\n'  )
            continue
        
    out_str = '\t' + token0 + ' ' + ' '.join([ "%f" % float(t) for t in tokens if isnumeric(t) ])
    
    outputfile.write(out_str + '\n')
    print line
    print out_str

outputfile.close()
inputfile.close()
