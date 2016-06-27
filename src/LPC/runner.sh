#bin/bash

var=0
while true; do
    var=`expr $var + 1`
    ./say $1 $var > /dev/dsp
    echo $var
    sleep 1
done
