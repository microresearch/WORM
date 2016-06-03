#bin/bash

var=0
while true; do
    var=`expr $var + 1`
    ./lap $var > /dev/dsp
    echo $var 
done
