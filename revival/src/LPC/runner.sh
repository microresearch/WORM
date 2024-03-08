#bin/bash

var=$2
while true; do
    ./say $var $1 | aplay
    echo "\n"
    echo $var
    sleep 1
    var=`expr $var + 1`
done
