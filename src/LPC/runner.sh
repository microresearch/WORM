#bin/bash

var=$2
while true; do
    var=`expr $var + 1`
    ./say $var $1 > /dev/dsp
    echo "\n"
    echo $var
    sleep 1
done
