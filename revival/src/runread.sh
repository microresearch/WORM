#bin/bash

var=$1
while true; do
    ./lpcr2 $var $2 $3 | aplay
    echo "\n"
    echo $var
    sleep 1
    var=`expr $var + 1`
done

