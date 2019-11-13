#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/crypto.hpp>

using namespace eosio;

CONTRACT rock_paper_scissors : public contract {
   public:
      using contract::contract;

      ACTION join( name player, asset wager );
      ACTION commit( name player, checksum256 digest);
      ACTION reveal( name player, std::string choice, std::string salt);
      using join_action = action_wrapper<"join"_n, &rock_paper_scissors::join>;
      using commit_action = action_wrapper<"commit"_n, &rock_paper_scissors::commit>;
      using reveal_action = action_wrapper<"reveal"_n, &rock_paper_scissors::reveal>;

};