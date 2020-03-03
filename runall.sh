#!/bin/bash
for i in `seq 0 11`;
do
    echo $1 $i
    ./$1 $i
done