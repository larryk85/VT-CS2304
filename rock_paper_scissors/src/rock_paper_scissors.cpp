#include <rock_paper_scissors.hpp>

// used to preinitialize some state
void rock_paper_scissors::bootstrap() {
    require_auth(get_self());
    _games.emplace(get_self(), [&](auto& g) {
        g.game_id = 0;
        g.clear_winners = {"foo"_n};
        g.wager_pool = {0, {"SYS", 4}};
        g.is_active = false;
    });
}

void rock_paper_scissors::update_player(name player, uint64_t game_id, const asset& wager) {
   const auto& iter = _players.find(player.value);
   if (iter == _players.end()) {
      _players.emplace(get_self(), [&](auto& gp) {
         gp.player = player;
         gp.games.push_back(game_id);
         gp.wins = 0;
         gp.losses = 0;
         gp.highest_wager = wager;
      });
   } else {
      _players.modify(iter, get_self(), [&](auto& gp) {
         gp.games.push_back(game_id);
         gp.highest_wager = gp.highest_wager > wager ? gp.highest_wager : wager;
      });
   }
}

void rock_paper_scissors::add_player_state(name player, uint64_t game_id, const checksum256& choice) {
   player_states pstates(this, game_id);
   if (!pstates.exists(player)) {
      pstates.emplace(player, [&](auto& e) {
         e.choice = choice;
      });
   }
}

// function to "deposit" funds to this account
// this should do the internal record keeping with
// the `_funds` table
void rock_paper_scissors::handle_deposit(const name& player, const asset& dep) {
   const auto& iter = _funds.find(player.value);
   if (iter == _funds.end()) {
      _funds.emplace(get_self(), [&](auto& f) {
         f.player  = player;
         f.deposit = dep;
      });
   } else {
      _funds.modify(iter, get_self(), [&](auto& f) {
         f.deposit += dep;
      });
   }
}

// use this action if this contract is deployed to the `eosio` account
void rock_paper_scissors::deposit( name player, asset dep ) {
    // you must validate the inputs
    // - this action should have the authorization of `player`
    // - `dep` should be non-negative
    check(!is_asset_negative(dep), "negative asset");
    require_auth(player);

    // the body of the deposit should be here
    handle_deposit(player, dep);

    // you will need to transfer the funds using the token contract
    token::transfer_action deposit_funds(eosio_token, add_authorizations(player));
    deposit_funds.send(player, get_self(), dep, "depositing funds");
}

// this action should return funds back to a player
void rock_paper_scissors::release( name player ) {
    // - this action should transfer funds back to the user
    // - update the book keeping `_funds` table and erase the entry
    // - extra credit *validate that the player is not currently in any active
    //   games
    const auto& iter = _funds.find(player.value);
    check(iter != _funds.end(), "no record exists");
    asset current_deposit = iter->deposit;

    // make sure you do more than just modify the _funds table, you need to transfer
    // the funds back with a token transfer too
    token::transfer_action release_funds(eosio_token, add_authorizations(player));
    release_funds.send(get_self(), player, current_deposit, "releasing funds");
}

void rock_paper_scissors::join( name player, asset wager, checksum256 choice ) {
    // you need to validate the inputs
    // - this action should have the authorization of `player`
    // - `player` should have enough tokens deposited in the game to
    //    cover the wager
    // - `wager` should be non-negative
    // - you will need to schedule any new game for later processing
    require_auth(player);
    const auto& fund_iter = _funds.find(player.value);
    check(fund_iter != _funds.end(), "player should have deposited funds");
    check(fund_iter->deposit >= wager, "player should have deposited enough funds to cover the wager");

    // grab the current block time
    time_point current_time = current_time_point();

    // helper lambda to show how to initialize some of the data types
    auto new_game = [&](uint64_t id) {
        _games.emplace(get_self(), [&](auto& g) {
           g.game_id = id;
           g.clear_winners.emplace_back(player);
           g.deadline = plus_five_minutes(current_time).sec_since_epoch();
           g.wager_pool = wager;
           g.is_active = true;
        });
        new_game_state(id);
        update_player(player, 1, wager);
        add_player_state(player, id, choice);
    };

    // get the last game id that was used
    uint64_t last_game_id = get_last_game_id();

    // go past the initial game to allow for easier logic for you all
    if (last_game_id == 0) {
        new_game(1);
    } else {
        // - you will need to check the games time to ensure that it has not
        //   gone over the deadline, then deactive that game when that is true
        // - if the game has gone over the deadline you need to notify all of
        //   the players of that game (require_recipient)
        //   - you will need to create a new game for the player
        //   - else you will need to modify the last game to include this player their wager
        const auto& game_to_join = _games.find(last_game_id);
        if (game_to_join->deadline < current_time.sec_since_epoch()) {
            _games.modify(game_to_join, get_self(), [&](auto& g) {
               g.is_active = false;
            });
           new_game(last_game_id + 1);
        } else {
           /* what needs to go here? */
        }
        rungame();
    }
}

// this will act as your main game loop
void rock_paper_scissors::rungame() {
    // first we need to grab the main game state object
    auto state = _state.get_or_default();
    time_point current_time = current_time_point();

    // as before you will need to check to see if the last game has expired
    // and mark it is_active as false and notify all the players of that game

    // while we still have matches to process, process them
    if (state.games_to_process.size()) {
        /* do your round processing here */
        // this should get a game from the games to process bag
        const auto& iter = _games.find(_state.get_or_default().games_to_process.front());
        game g = *iter;
        uint32_t N = 3; // you should tweak this number to see what is optimal
        if (have_all_players_revealed(g.game_id)) {
            for (uint32_t i=0; i < N; i++) {
                // for some amount of iterations work through the rounds
                // of this game and keep whittling at the winners set
                // until only 1 winner is left
            }
        }
    }
}

// this action will validate that the player is giving the right choice that matches their commitment
void rock_paper_scissors::reveal( uint64_t game_id, name player, std::string choice, std::string salt ) {
    std::string choice_and_salt = choice+salt;
    checksum256 chksum_choice = sha256(choice_and_salt.c_str(), choice_and_salt.length());

    player_states pstates(this, game_id);

    const auto& p = pstates.get(player);

    check(chksum_choice == p.choice, "player's revealed answer does not match");

    // you will need to validate that the player's chksum_choice does indeed match their
    // committed answer from `join`

    // you will need to mark the player an inactive if they reveal before the game
    // has been retired, i.e. game_iter->is_active == false
    // any player who's status is active == false loses by default when determining
    // the winner

    // you will need to modify the table to store the players string choice
    // this will allow you to determine the winner in rungame
}

// use this notification handler deposit approach if
// this contract is deployed to the `rps` account
void rock_paper_scissors::on_deposit(name from, name to, asset funds, std::string memo) {
    // this will keep the notification handler from firing off and doing duplicated work
    if (get_self() == "eosio"_n)
        return;

    // - validate that `to` is the same as the account this contract is deployed to
    // - validate that the funds are positive
    // - require the authorization of `from`
    // - the rest of the body of this should be the same as the `eosio`
    //   account deposit action except the inline transfer
    check(!is_asset_negative(funds), "negative asset");
    require_auth(from);
    check(to == get_self(), "to should be get_self()");

    // body of handling the deposit should be here
}
