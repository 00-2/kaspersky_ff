#!/bin/bash
if [[ $# -eq 0 ]] ; then
    echo "Wrong count of element. Required>=1 : Received: "$#
    exit 0
fi
to_find=$1
filename=$PWD
if [[ $# -ge 2 ]] ; then
    filename=$2
fi
echo "$(sudo /usr/lib/ff/ff_bin $to_find $filename)"
if ! [ "$res" = "" ]; then
  echo "$res"
fi