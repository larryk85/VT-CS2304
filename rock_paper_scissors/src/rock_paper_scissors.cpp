#include <rock_paper_scissors.hpp>

void rock_paper_scissors::bootstrap() {
    require_auth(get_self());
    _games.emplace(get_self(), [&](auto& g) {
        g.game_id = 0;
        g.players = {};
        g.clear_winners = {};
        g.wager_pool = {};
        g.is_active = false;
    });
}
// function to "deposit" funds to this account
// this should do the internal record keeping with
// the `_funds` table
void rock_paper_scissors::handle_deposit(const name& player, const asset& dep) {
    // body of internal book keeping
}

// use this action if this contract is deployed to the `eosio` account
void rock_paper_scissors::deposit( name player, asset dep ) {
    // you must validate the inputs
    // - this action should have the authorization of `player`
    // - `dep` should be non-negative

}

// this action should return funds back to a player
void rock_paper_scissors::release( name player ) {
    // - this action should transfer funds back to the user
    // - update the book keeping `_funds` table and erase the entry
    // - validate that the player is not currently in any active
    //   games
}

void rock_paper_scissors::join( name player, asset wager, checksum256 choice ) {
    // you need to validate the inputs
    // - this action should have the authorization of `player`
    // - `player` should have enough tokens deposited in the game to
    //    cover the wager
    // - `wager` should be non-negative
}

void rock_paper_scissors::rungame() {

}

void rock_paper_scissors::reveal( uint64_t game_id, name player, std::string choice, std::string salt ) {

}

// use this notification handler deposit approach if 
// this contract is deployed to the `rps` account
void rock_paper_scissors::on_deposit(name from, name to, asset funds, std::string memo) {
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
}