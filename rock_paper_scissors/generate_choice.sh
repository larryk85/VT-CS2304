#! /bin/bash

choice_cmd="echo -n $1$2 | sha256sum | cut -d \" \" -f1"
choice=`eval $choice_cmd`
echo $choice