#!/bin/bash
#read 2 i2c tmp101 sensors from TI and print the values on stdout ... 
declare -i tmp1 #declare -i defines tmp1 as integer :)
declare -i tmp2
while [ 1 ]; do
   tmp1=`i2cget -y 1 0x49 0` 
   tmp2=`i2cget -y 1 0x48 0` 
   tmf1=`echo "scale=3;$tmp1/0.555+32" | bc`
   tmf2=`echo "scale=3;$tmp2/0.555+32" | bc`
   echo "0x49 has the temperature: $tmp1 C° = $tmf1 F"
   echo "0x48 has the temperature: $tmp2 C° = $tmf2 F"
sleep 1
done

