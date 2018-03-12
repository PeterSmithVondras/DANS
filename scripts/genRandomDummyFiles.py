import time
from random import random
import os
import sys

if len(sys.argv) != 5:
  print 'usage: python genDummyFiles.py blockSize blockCount startIDX fileCount'
  exit(0)

blkSize = sys.argv[1]
blkCount = sys.argv[2]
startIdx = sys.argv[3]
fCount = sys.argv[4]

for idx in range(int(startIdx),int(fCount)+int(startIdx)):
  filename = "F"+str(idx)
  cmd = 'dd if=/dev/random of='+str(filename)+' bs=' +str(blkSize)+' count='+str(blkCount)
  os.system(cmd)
