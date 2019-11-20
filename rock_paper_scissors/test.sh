#! /bin/bash

function check_state() {
    cleos get table $ACCOUNT $ACCOUNT $1 &> .test.output
    diff -I "game_deadline.*" .test.output ./expected/$1.$2.expected &> /dev/null
    status=$?
    if [[ $status != 0 ]]; then
        echo "Failure, funds table does not match $1.$2.expected"
        diff -I "game_deadline.*" .test.output ./expected/$1.$2.expected
        exit -1
    fi
}

if [[ $1 == "non-root" ]]; then
    ACCOUNT="rps"
    is_eosio=false
else
    ACCOUNT="eosio"
    is_eosio=true
fi

export ACCOUNT=$ACCOUNT

pushd ../
./stop.sh
./reset.sh
./start.sh &> /dev/null &
sleep 2
./bootstrap.sh
popd

./build.sh

if [ "$is_eosio" = true ]; then
    cleos push action $ACCOUNT deposit '["player1", "100.0000 VT"]' -p player1
    cleos push action $ACCOUNT deposit '["player2", "100.0000 VT"]' -p player2
    cleos push action $ACCOUNT deposit '["player3", "100.0000 VT"]' -p player3
    cleos push action $ACCOUNT deposit '["player4", "100.0000 VT"]' -p player4
    cleos push action $ACCOUNT deposit '["player5", "100.0000 VT"]' -p player5
else
    cleos push action eosio.token transfer '["player1", "rps", "100.0000 VT", "depositing"]' -p player1
    cleos push action eosio.token transfer '["player2", "rps", "100.0000 VT", "depositing"]' -p player2
    cleos push action eosio.token transfer '["player3", "rps", "100.0000 VT", "depositing"]' -p player3
    cleos push action eosio.token transfer '["player4", "rps", "100.0000 VT", "depositing"]' -p player4
    cleos push action eosio.token transfer '["player5", "rps", "100.0000 VT", "depositing"]' -p player5
fi

check_state funds 0

cleos push action $ACCOUNT join '["player1", "10.0000 VT"]' -p player1
cleos push action $ACCOUNT join '["player2", "10.0000 VT"]' -p player2
cleos push action $ACCOUNT join '["player3", "10.0000 VT"]' -p player3

check_state games 0

sleep 3

cleos push action $ACCOUNT join '["player4", "33.0000 VT"]' -p player4
cleos push action $ACCOUNT join '["player1", "20.0000 VT"]' -p player1
cleos push action $ACCOUNT join '["player5", "23.0000 VT"]' -p player5

check_state games 1