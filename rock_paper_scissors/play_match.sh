#! /bin/bash

player=$1
wager="\"$2\""
commitment=$3

cleos push action rps join "["$player", $wager]" -p $1
hash=`printf "$commitment" | openssl dgst -r -sha256 | { read first sha ; echo $first ; }`
cleos push action rps commit "[\"$hash\"]" -p $1