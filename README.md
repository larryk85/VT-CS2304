# VT-CS2304

* Sign up for dockerhub (https://www.hub.docker.com)
* Install Docker Desktop, should automatically redirect you to this after sign up.
    * Open either Terminal or Powershell (for Mac OSX or Windows)
    * Type the command `git clone https://github.com/larryk85/VT-CS2304`
    * Type the command `docker build eosio ./VTCS2304`
    * Now you can start a running instance of nodeos (the eosio blockchain node software).
      * Type the command `docker run -p 8888:8888 -p5555:5555 nodeos -e -p eosio --plugin=eosio::http_plugin --plugin=eosio::net_api_plugin`
      * You can now run `docker ps` to see the containers you have running, this will also have the container name for your running instance.
      * To stop nodeos (this will destroy anything blocks log information), run the command `docker stop <container name>`
* Download EOSStudio (https://www.eosstudio.io)
    * On start up it should ask you to download EOSIO, EOSIO.CDT and EOSIO.Contracts, install the latest version of each of these.
    * Under the network tab select `local` and start nodeos.
    * If you cannot start nodeos via EOSStudio, then simply start nodeos via docker and change the network to custom and set the address to `localhost:8888`.
    * Next compile the new project that you have created (ctrl-b).
    * Now you can deploy this to the running nodeos instance, for the time being you can deploy to the `eosio` account and not have to create your own account.
* Install geth (https://geth.ethereum.org/downloads)
   * Also click on install `development tools`.
* Install nodejs (https://nodejs.org/en/download)
* Install truffle via npm (npm install truffle -g)
   * Make a new directory for a test contract.
   * cd into the new directory and run the command `truffle init`
   * Then run `truffle compile`.
* Install ganache (https://www.trufflesuite.com/ganache)
   * Create a new workspace.
   * Add the truffle project that we just created.
   * Save the workspace.
   * Back at the terminal in the directory we created for truffle run `truffle deploy`.
