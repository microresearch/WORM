#bin/bash

var=$2
while true; do
    ./say $1 $var | aplay
    echo "\n"
    echo $var
    sleep 1
    var=`expr $var + 1`
done
