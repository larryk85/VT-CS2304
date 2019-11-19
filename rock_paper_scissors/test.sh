#! /bin/bash

pushd ../
./stop.sh
./reset.sh
./start.sh &> /dev/null &
sleep 2
./bootstrap.sh
popd

./build.sh
cleos push action rps deposit '["player1", "100.0000 VT"]' -p player1
cleos push action rps deposit '["player2", "100.0000 VT"]' -p player2
cleos push action rps deposit '["player3", "100.0000 VT"]' -p player3

cleos get table rps rps funds &> .test.output
cmp ".test.output" "funds.0.expected"
status=$?
if [[ $status != 0 ]]; then
    echo "Failure, funds table does not match funds.0.expected"
    diff .test.output funds.0.expected
    exit -1
fi

cleos push action rps join '["player1", "10.0000 VT"]' -p player1
cleos push action rps join '["player2", "10.0000 VT"]' -p player2
cleos push action rps join '["player3", "10.0000 VT"]' -p player3

cleos get table rps rps games &> .test.output
diff -I "game_deadline.*" ".test.output" "games.0.expected" &> /dev/null
status=$?
if [[ $status != 0 ]]; then
    echo "Failure, games table does not match games.0.expected"
    diff -I "game_deadline.*" .test.output games.0.expected
    exit -1
fi