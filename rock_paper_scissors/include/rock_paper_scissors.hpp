#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/crypto.hpp>
#include <eosio/system.hpp>
#include <eosio/singleton.hpp>

#include <eosio.token/eosio.token.hpp>

#include <map>
#include <set>
#include <limits>
#include <vector>
#include <queue>

using namespace eosio;

// To use this table as a replacement for the map in game
// create an object when you need `player_states pstates(this, <game_id>)`, this will create a separate table per game id
// emplace/modify somewhat like normal `pstates.emplace(player, [&](auto& e){e.player = player; ...`, I have made the emplace and modify in such (note there is no need to initialize the player field).
// a way that all you need to pass in is the key (player) and the lambda that will initialize or modify the object
// a getter `pstate.get(player)` is given to have access to an object if you just need to read the player_state object
// you can erase by simply calling `pstate.erase(player)`
// to apply something or do some action over every element you can call for_each([&](auto& e) { ... logic in here ... });
// finally, a helper method `pstates.exists(player)` is given to determine if the row already exists or not
//
// See in choose_loser for an example of using this
template <typename Owner>
class player_map {
   public:
     struct [[eosio::table, eosio::contract("rock_paper_scissors")]] player_state {
         name player;
         checksum256 choice;
         std::string actual_choice;
         bool active = true;
         uint64_t primary_key()const { return player.value; }
     };

      player_map(Owner* owner, uint64_t game_id) : _owner(owner), _players(owner->get_self(), game_id) {}

      template <typename F>
      void emplace(name player, F&& emplace_func) {
         check(_players.find(player.value) == _players.end(), "trying to emplace a player_state that already exists");
         _players.emplace(_owner->get_self(), [&](auto& e) {
            e.player = player;
            emplace_func(e);
         });
      }

      template <typename F>
      void modify(name player, F&& modify_func) {
         const auto& iter = _players.find(player.value);
         check(iter != _players.end(), "trying to modify player_state that doesn't exist");
         _players.modify(iter, _owner->get_self(), [&](auto& e) { modify_func(e); });
      }

      bool exists(name player)const { return _players.find(player.value) != _players.end(); }

      void erase(name player) {
         const auto& iter = _players.find(player.value);
         check(iter != _players.end(), "player_state doesn't exist");
         _players.erase(_owner->get_self(), iter);
      }

      const player_state& get(name player)const {
         const auto& iter = _players.find(player.value);
         check(iter != _players.end(), "player_state doesn't exist");
         return *(_players.find(player.value));
      }

      template <typename F>
      void for_each(F&& f) { for (auto& e : _players) f(e); }

      using players = multi_index<"players"_n, player_state>;
   private:
      Owner*  _owner;
      players _players;
};

static inline auto default_auths = std::vector<eosio::permission_level>{};
time_point plus_five_minutes(const time_point& p) {
    return static_cast<time_point>(time_point_sec(1)) + p;
}

bool is_asset_negative(const asset& a) { return a.amount < 0; }

permission_level create_permission_level(name nm) { return {nm, "active"_n}; }
template <typename... Ts>
std::vector<permission_level> add_authorizations(Ts... names) {
    return {create_permission_level(names)...};
}

inline constexpr auto eosio_token = "eosio.token"_n;

CONTRACT rock_paper_scissors : public contract {
   private:
        TABLE game {
            uint64_t game_id;
            name last_eval_player;
            uint32_t deadline;
            asset wager_pool;
            bool is_active;
            //std::map<name, player_state> players;
            std::vector<name> clear_winners;
            uint64_t primary_key()const { return game_id; }
        };

        TABLE gamestate {
            uint64_t last_game_id;
            std::vector<uint64_t> games_to_process;
            uint32_t iteration;
            uint64_t primary_key()const { return 0; }
        };

        TABLE gameplayer {
            name player;
            std::vector<uint64_t> games;
            // fields used for metrics later
            uint64_t wins;
            uint64_t losses;
            asset highest_wager;
            uint64_t primary_key()const { return player.value; }
            uint64_t by_wins()const { return wins; }
            uint64_t by_losses()const { return losses; }
            uint64_t by_wager()const { return highest_wager.amount; }
        };

        TABLE fund {
            name player;
            asset deposit;
            uint64_t primary_key()const { return player.value; }
        };
   public:
        rock_paper_scissors(name s, name c, datastream<const char*> ds)
          : _games(s, s.value),
            _funds(s, s.value),
            _players(s, s.value),
            _state(s, s.value),
            contract(s, c, ds) {}

        ACTION bootstrap();
        ACTION deposit( name player, asset dep);
        ACTION release( name player );

        ACTION join( name player, asset wager, checksum256 choice );
        ACTION reveal( uint64_t game_id, name player, std::string choice, std::string salt);
        ACTION rungame();

        [[eosio::on_notify("eosio.token::transfer")]]
        void on_deposit(name from, name to, asset funds, std::string memo);


        using join_action = action_wrapper<"join"_n, &rock_paper_scissors::join>;
        using reveal_action = action_wrapper<"reveal"_n, &rock_paper_scissors::reveal>;

        using games = multi_index<"games"_n, game>;
        using funds = multi_index<"funds"_n, fund>;
        using game_players = multi_index<"gameplayers"_n, gameplayer,
            indexed_by<"bywins"_n, const_mem_fun<gameplayer, uint64_t, &gameplayer::by_wins>>,
            indexed_by<"bylosses"_n, const_mem_fun<gameplayer, uint64_t, &gameplayer::by_losses>>,
            indexed_by<"bywager"_n, const_mem_fun<gameplayer, uint64_t, &gameplayer::by_wager>>>;
        using game_state = singleton<"gamestate"_n, gamestate>;
        using player_states = player_map<rock_paper_scissors>;

    private:

        // function to "deposit" funds to this account
        // this should do the internal record keeping with
        // the `_funds` table
        void handle_deposit(const name& player, const asset& dep);
        void update_player(name player, uint64_t game_id, const asset& wager);
        void add_player_state(name player, uint64_t game_id, const checksum256& choice);

        template <typename Iter>
        Iter choose_loser(game& g, Iter first, Iter second) {
            player_states pstates(this, g.game_id);

            auto ps1 = pstates.get(*first);
            auto ps2 = pstates.get(*second);
            std::string first_choice = ps1.actual_choice;
            std::string second_choice = ps2.actual_choice;

            // handling the case where the player is inactive
            if (!ps1.active)
                return first;
            else if (!ps2.active)
                return second;

            // you need to determine the loser of this round,
            // regular rock, paper, scissors rules apply
        }

        void schedule_for_processing(uint64_t id) {
            auto _s = _state.get_or_default();
            _s.games_to_process.push_back(id);
            _state.set(_s, get_self());
        }

        void remove_from_processing(gamestate&& state) {
            state.games_to_process.pop_back();
            _state.set(state, get_self());
        }

        void new_game_state(uint64_t id) {
            auto _s = _state.get_or_default();
            _s.last_game_id = id;
            _state.set(_s, get_self());
        }

        uint64_t get_last_game_id() {
            return _state.get_or_default().last_game_id;
        }

        // helper method to determine if all of the players
        // have revealed their answers
        bool have_all_players_revealed( uint64_t game_id ) {
            const auto& iter = _games.find(game_id);
            player_states pstates(this, game_id);

            bool all_revealed = true;
            pstates.for_each([&](auto& e) {
               if (e.actual_choice.empty())
                  all_revealed = false;
            });
            return all_revealed;
        }

        games _games;
        funds _funds;
        game_players _players;
        game_state _state;
};
