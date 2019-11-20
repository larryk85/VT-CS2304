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

// Given the semantics of upper_bound(...), if the upper bound
// does not exist then it will return the last element.
// This is a variable that you can use to grab the last element
// in your table.
static inline uint64_t game_upper_bound = std::numeric_limits<uint64_t>::max();

static inline auto default_auths = std::vector<eosio::permission_level>{};
time_point to_seconds_minutes(const uint32_t t) {
    return static_cast<time_point>(time_point_sec(t));
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
        struct player_state {
            player_state() = default;
            player_state(name player) : player(player) {}
            player_state(name player, checksum256 choice) : player(player), choice(choice) {}
            name player;
            checksum256 choice;
            std::string actual_choice;
            bool active = true;
            EOSLIB_SERIALIZE(player_state, (player)(choice)(active));
        };

        TABLE game {
            uint64_t game_id;
            std::map<name, player_state> players;
            std::vector<name> clear_winners;
            name last_eval_player;
            time_point game_deadline;
            asset wager_pool;
            bool is_active;
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
            std::set<uint64_t> games;
            // fields used for metrics later
            uint32_t wins;
            uint32_t losses;
            asset highest_wager;
            uint64_t primary_key()const { return player.value; }
            uint32_t by_wins()const { return wins; }
            uint32_t by_losses()const { return losses; }
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
            indexed_by<"bywins"_n, const_mem_fun<gameplayer, uint32_t, &gameplayer::by_wins>>,
            indexed_by<"bylosses"_n, const_mem_fun<gameplayer, uint32_t, &gameplayer::by_losses>>,
            indexed_by<"bywager"_n, const_mem_fun<gameplayer, uint64_t, &gameplayer::by_wager>>>;
        using game_state = singleton<"gamestate"_n, gamestate>;

    private:
 
        // function to "deposit" funds to this account
        // this should do the internal record keeping with
        // the `_funds` table
        void handle_deposit(const name& player, const asset& dep);

        void run_round(game& g);

        void run_rounds(uint32_t rounds);

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

        bool has_player_committed_choice( const player_state& p ) {
            static const auto default_choice = checksum256{};
            return p.choice == default_choice;
        }
        
        bool have_all_players_committed( uint64_t game_id ) {
            const auto& iter = _games.find(game_id);
            for (const auto& [k, p_state] : iter->players)
                if (!has_player_committed_choice( p_state))
                    return false;
            return true;
        }

        games _games;
        funds _funds;
        game_players _players;
        game_state _state;
};