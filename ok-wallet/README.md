### 编译

1. cd raiblocks/ok-wallet
- mkdir build
- cd build
- cmake -DACTIVE_NETWORK=rai_live_network ..   ## for testnet, run cmake -DACTIVE_NETWORK=rai_test_network ..
- make
- ls *.so
- ls *.dylib

### 注意项
- raiblocks的测试网是一个单点网络，具体创建方法参见[raiblocks issues443](https://github.com/nanocurrency/raiblocks/issues/443)。简单来说，你需要使用 ACTIVE_NETWORK=rai_test_network 作为参数编译项目。在项目中已经提供了一个 test_genesis_key 作为测试账号使用，此账号提供了无数的币值用来测试（具体可参见rai/common.cpp）。

- 热钱包中，使用rpc协议中的"wallet_add_watch"添加账号，这样在热钱包中只有账号地址，没有私钥。但在测试代码中，为了方便我直接在钱包中添加了一个私钥。

- 使用正式网时（编译时ACTIVE_NETWORK为rai_live_network），调用produceUnsignedTx(或Java_com_okcoin_vault_jni_raiblocks_Raiblocksj_execute中的"createrawtransaction命令")时，都会先去更新本地的账本数据库。如果本地库比较旧，会比较耗时。

- 在raiblocks的代码中，并没有一个节点可以确切的知道“到此为止 我的账本数据更新完了”。相反，当没有数据更新时系统会一直等待（参见bootstrap_attempt::run）。我的做法是连续等待10秒以上，我就认为更新结束了（参见AppWrapper::waitfor_catchup_ledger）。
