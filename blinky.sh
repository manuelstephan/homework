#!/bin/bash
 
# getting leds timed
echo timer > /sys/class/leds/beaglebone:green:usr0/trigger
echo timer > /sys/class/leds/beaglebone:green:usr1/trigger
echo timer > /sys/class/leds/beaglebone:green:usr2/trigger
echo timer > /sys/class/leds/beaglebone:green:usr3/trigger


