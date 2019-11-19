#! /bin/bash

priv_key="5JPJoZXizFVi19wHkboX5fwwEU2jZVvtSJpQkQu3uqgNu8LNdQN"
pub_key="EOS6v86d8DAxjfGu92CLrnEzq7pySpVWYV2LjaxPaDJJvyf9Vpx5R"

cleos wallet import --private-key 5JPJoZXizFVi19wHkboX5fwwEU2jZVvtSJpQkQu3uqgNu8LNdQN
cleos create account eosio eosio.token EOS6v86d8DAxjfGu92CLrnEzq7pySpVWYV2LjaxPaDJJvyf9Vpx5R
cleos create account eosio rps EOS6v86d8DAxjfGu92CLrnEzq7pySpVWYV2LjaxPaDJJvyf9Vpx5R
cleos create account eosio player1 EOS6v86d8DAxjfGu92CLrnEzq7pySpVWYV2LjaxPaDJJvyf9Vpx5R
cleos create account eosio player2 EOS6v86d8DAxjfGu92CLrnEzq7pySpVWYV2LjaxPaDJJvyf9Vpx5R
cleos create account eosio player3 EOS6v86d8DAxjfGu92CLrnEzq7pySpVWYV2LjaxPaDJJvyf9Vpx5R
cleos create account eosio player4 EOS6v86d8DAxjfGu92CLrnEzq7pySpVWYV2LjaxPaDJJvyf9Vpx5R
cleos create account eosio player5 EOS6v86d8DAxjfGu92CLrnEzq7pySpVWYV2LjaxPaDJJvyf9Vpx5R
cleos create account eosio playera EOS6v86d8DAxjfGu92CLrnEzq7pySpVWYV2LjaxPaDJJvyf9Vpx5R
cleos create account eosio playerb EOS6v86d8DAxjfGu92CLrnEzq7pySpVWYV2LjaxPaDJJvyf9Vpx5R
cleos create account eosio playerc EOS6v86d8DAxjfGu92CLrnEzq7pySpVWYV2LjaxPaDJJvyf9Vpx5R
cleos create account eosio playerd EOS6v86d8DAxjfGu92CLrnEzq7pySpVWYV2LjaxPaDJJvyf9Vpx5R
cleos create account eosio playere EOS6v86d8DAxjfGu92CLrnEzq7pySpVWYV2LjaxPaDJJvyf9Vpx5R

cleos set account permission eosio.token active --add-code
cleos set account permission rps active --add-code

cleos set code eosio.token eosio.token.wasm
cleos set abi eosio.token eosio.token.abi

cleos push action eosio.token create '["eosio", "100000.0000 VT"]' -p eosio.token

cleos push action eosio.token issue '["player1", "10000.0000 VT", "issuing tokens"]' -p eosio
cleos push action eosio.token issue '["player2", "10000.0000 VT", "issuing tokens"]' -p eosio
cleos push action eosio.token issue '["player3", "10000.0000 VT", "issuing tokens"]' -p eosio
cleos push action eosio.token issue '["player4", "10000.0000 VT", "issuing tokens"]' -p eosio
cleos push action eosio.token issue '["player5", "10000.0000 VT", "issuing tokens"]' -p eosio
cleos push action eosio.token issue '["playera", "10000.0000 VT", "issuing tokens"]' -p eosio
cleos push action eosio.token issue '["playerb", "10000.0000 VT", "issuing tokens"]' -p eosio
cleos push action eosio.token issue '["playerc", "10000.0000 VT", "issuing tokens"]' -p eosio
cleos push action eosio.token issue '["playerd", "10000.0000 VT", "issuing tokens"]' -p eosio
cleos push action eosio.token issue '["playere", "10000.0000 VT", "issuing tokens"]' -p eosio