#!/bin/sh
# From http://wh1t3s.com/2009/05/14/reading-beagleboard-gpio/
#
# Read a GPIO input

GPIO=$1

cleanup() { # Release the GPIO port
  echo $GPIO > /sys/class/gpio/unexport
  echo ""
  echo ""
  exit
}

# Open the GPIO port
#
echo "$GPIO" > /sys/class/gpio/export
echo "in" > /sys/class/gpio/gpio${GPIO}/direction
echo 60 > /sys/class/gpio/export
echo out > /sys/class/gpio/gpio60/direction
# exporting gpio 60
trap cleanup SIGINT # call cleanup on Ctrl-C

THIS_VALUE=`cat /sys/class/gpio/gpio${GPIO}/value`
LAST_VALUE=$THIS_VALUE
NEWLINE=0

# Read forever

while [ "1" = "1" ]; do
  # next three lines detect state transition
  if [ "$THIS_VALUE" != "$LAST_VALUE" ]; then
    EV="|"
  else
    EV=""
  fi

  # "^" for high, '_' for low
  if [ "1" = "$THIS_VALUE" ]; then
    EV="${EV}^"
  echo 1 > /sys/class/gpio/gpio60/value
  else
    EV="${EV}_"
  echo 0 > /sys/class/gpio/gpio60/value
  fi
  echo -n $EV

  # sleep for a while
  sleep 0.05

  # wrap line every 72 samples
  LAST_VALUE=$THIS_VALUE
  THIS_VALUE=`cat /sys/class/gpio/gpio${GPIO}/value`
  NEWLINE=`expr $NEWLINE + 1`
  if [ "$NEWLINE" = "72" ]; then
    echo ""
    NEWLINE=0
  fi

done

cleanup # call the cleanup routine

