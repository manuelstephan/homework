#!/bin/bash
#read temp
declare -i tmp1
tmp1=`i2cget -y 1 0x48 0`  
tmf1=`echo "scale=3;$tmp1/0.555+32" | bc`
echo "0x48 has the temperature: $tmp1 C° = $tmf1 F"
