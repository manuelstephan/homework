#!/bin/bash
# @author:Manuel Stephan
# i2cget -y 1 0x48 0x03 w, read value of the upper threashold of the tempsensor 
# 1 is bus number, 0x48 is address of device 0x03 is subaddress, w means word 
i2cset -y 1 0x048 0x03 0x20 # set upper threshold of 0x48
i2cset -y 1 0x049 0x03 0x20 # set upper threshold of 0x49

i2cset -y 1 0x048 0x02 0x0f # set lower threshold of 0x48
i2cset -y 1 0x049 0x02 0x0f # set lower threshold of 0x49

