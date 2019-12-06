#! /bin/bash

# set this to true if you want verbose output
VERBOSE=true

if [[ $1 == "non-root" ]]; then
    ACCOUNT="rps"
    is_eosio=false
else
    ACCOUNT="eosio"
    is_eosio=true
fi

export ACCOUNT=$ACCOUNT

if [[ "$VERBOSE" = true ]]; then
    output_stream="2>&1"
else
    output_stream="&> /dev/null"
fi

function check_state() {
    if [[ $# -eq 3 ]]; then
       SCOPE=$3
    else
       SCOPE=$ACCOUNT
    fi
    command="cleos --verbose get table $ACCOUNT $SCOPE $1"
    eval $command &> .test.output
    diff -I "deadline.*" -I "warn.*" -I "executed.*" -I "\"next_key.*" .test.output ./expected/$1.$2.expected &> /dev/null
    status=$?
    if [[ $status != 0 ]]; then
        echo -e "\e[31mFailure, table does not match $1.$2.expected\e[0m"
        diff -I "deadline.*" .test.output ./expected/$1.$2.expected
        exit -1
    fi
    echo -e "\e[32mcheck state $1 $2 passed\e[0m"
}

function try_deposit() {
    if [ "$is_eosio" = true ]; then
        command="cleos --verbose push action $ACCOUNT deposit '[\"$1\", \"$2\"]' -p $1"
    else
        command="cleos --verbose push action eosio.token transfer '[\"$1\", \"rps\", \"$2\", \"depositing funds\"]' -p $1"
    fi
    eval $command $output_stream
    status=$?
    if [[ $status != 0 ]]; then
        echo -e "\e[31mFailed deposit $1\e[0m"
        exit -1
    fi
    echo -e "\e[32mDeposit $1 $2 passed\e[0m"
}

function try_join() {
    choice_cmd="echo -n $3$4 | sha256sum | cut -d \" \" -f1"
    choice=`eval $choice_cmd`
    command="cleos --verbose push action $ACCOUNT join '[\"$1\", \"$2\", \"$choice\"]' -p $1"
    eval $command #$output_stream
    status=$?
    if [[ $status != 0 ]]; then
        echo -e "\e[31mFailed to join $1\e[0m"
        exit -1
    fi
    echo -e "\e[32mJoining a game $1 $2 passed\e[0m"
}

function try_reveal() {
    command="cleos --verbose push action $ACCOUNT reveal '[\"$1\", \"$2\", \"$3\", \"$4\"]' -p $2"
    eval $command #$output_stream
    status=$?
    if [[ $status != 0 ]]; then
        echo -e "\e[31mFailed to reveal $1\e[0m"
        exit -1
    fi
    echo -e "\e[32mRevealing $1 $2 passed\e[0m"
}

function try_rungame() {
    command="cleos --verbose push action $ACCOUNT rungame '[]' -p $ACCOUNT"
    eval $command #$output_stream
    status=$?
    if [[ $status != 0 ]]; then
        echo -e "\e[31mFailed to rungame\e[0m"
        exit -1
    fi
    echo -e "\e[32mRungame passed\e[0m"
}

function try_bootstrap() {
    command="cleos --verbose push action $ACCOUNT bootstrap '[]' -p $ACCOUNT"
    eval $command $output_stream
    status=$?
    if [[ $status != 0 ]]; then
        echo -e "\e[31mFailed to bootstrap\e[0m"
        exit -1
    fi
    echo -e "\e[32mBootstrapping passed\e[0m"
}

pushd ../
./stop.sh $output_stream
./reset.sh $output_stream
./start.sh &> /dev/null &
sleep 2
./bootstrap.sh &> /dev/null
popd

./build.sh

try_bootstrap

try_deposit player1 "100.0000 VT"
try_deposit player2 "100.0000 VT"
try_deposit player3 "100.0000 VT"
try_deposit player4 "100.0000 VT"
try_deposit player5 "100.0000 VT"

check_state funds 0

try_join player1 "10.0000 VT" "rock" 42
try_join player2 "10.0000 VT" "scissors" 33
try_join player3 "10.0000 VT" "paper" 13

check_state games 0
check_state players 0 1

sleep 3

try_join player4 "33.0000 VT" "paper" 3
try_join player1 "20.0000 VT" "rock" 32
try_join player5 "23.0000 VT" "paper" 22

check_state games 1
check_state players 1 2

sleep 3

try_reveal 1 player1 "rock" 42
try_reveal 1 player3 "paper" 13
try_reveal 1 player2 "scissors" 33

try_rungame
sleep 1
try_rungame
