 #!/bin/bash   
# configure the bone in heartbeat pattern
# used to reset it after experimenting with leds 

echo heartbeat > /sys/class/leds/beaglebone:green:usr0/trigger
echo heartbeat > /sys/class/leds/beaglebone:green:usr1/trigger
echo heartbeat > /sys/class/leds/beaglebone:green:usr2/trigger
echo heartbeat > /sys/class/leds/beaglebone:green:usr3/trigger
