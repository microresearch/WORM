#bin/bash

var=$2
while true; do
    var=`expr $var + 1`
    ./say $1 $var | aplay
    echo "\n"
    echo $var
    sleep 1
done
