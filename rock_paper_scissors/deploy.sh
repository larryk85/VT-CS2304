#! /bin/bash

if [[ -n "$ACCOUNT" ]]; then
   ACCOUNT="$ACCOUNT"
else
   ACCOUNT="eosio"
fi

cleos set code $ACCOUNT bin/rock_paper_scissors/rock_paper_scissors.wasm
cleos set abi $ACCOUNT bin/rock_paper_scissors/rock_paper_scissors.abi