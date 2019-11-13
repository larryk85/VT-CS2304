project(rock_paper_scissors)

set(EOSIO_WASM_OLD_BEHAVIOR "Off")
find_package(eosio.cdt)

add_contract( rock_paper_scissors rock_paper_scissors rock_paper_scissors.cpp )
target_include_directories( rock_paper_scissors PUBLIC ${CMAKE_SOURCE_DIR}/../include )
target_ricardian_directory( rock_paper_scissors ${CMAKE_SOURCE_DIR}/../ricardian )