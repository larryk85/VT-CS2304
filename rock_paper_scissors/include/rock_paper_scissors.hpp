#include <eosio/eosio.hpp>
using namespace eosio;

CONTRACT rock_paper_scissors : public contract {
   public:
      using contract::contract;

      ACTION hi( name nm );

      using hi_action = action_wrapper<"hi"_n, &rock_paper_scissors::hi>;
};