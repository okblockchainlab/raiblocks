#include "gtest/gtest.h"
#include "wallet.h"
#include <dlfcn.h>
#include <regex>


typedef bool (*get_address_t)(const std::string& prv, std::string& address);
typedef bool (*produce_unsigned_tx_t)(const std::string& from, const std::string& to, const std::string& amount, const std::string& net_type, const char* data_dir, std::string& utx);
typedef bool (*sign_transaction_t)(const std::vector<uint8_t>& utx, const std::string& seed, const std::string& net_type, const char* data_dir, std::vector<uint8_t>& stx);
typedef bool (*test_init_t)(const std::string& test_prv1, const std::string& test_prv2, std::string& data_dir);
typedef bool (*commit_transaction_t)(const std::vector<uint8_t>& stx, const std::string& net_type, const char* data_dir, std::string& result_str);

class OKWalletTest : public ::testing::Test {
public:
  static void SetUpTestCase() {
    //FIXME: use a better path
    wlt_mod = dlopen("./libraiblocks.dylib", RTLD_LAZY);
    getAddress = (get_address_t)dlsym(wlt_mod, "GetAddressFromPrivateKey");
    produceUnsignedTx = (produce_unsigned_tx_t)dlsym(wlt_mod, "produceUnsignedTx");
    signTransaction = (sign_transaction_t)dlsym(wlt_mod, "signTransaction");

    testInit = (test_init_t)dlsym(wlt_mod, "testInit");
    commitTransaction = (commit_transaction_t)dlsym(wlt_mod, "commitTransaction");

    ASSERT_TRUE(testInit(test_genesis_prv, test_prv2, data_dir));
  }

  static void TearDownTestCase() {
    if (NULL != wlt_mod) {
      dlclose(wlt_mod);
      wlt_mod = NULL;
    }

    system(("rm -rf " + data_dir).c_str());
  }


  virtual void SetUp() {
  }

  virtual void TearDown() {
  }

  static void* wlt_mod;
  static get_address_t getAddress;
  static produce_unsigned_tx_t produceUnsignedTx;
  static sign_transaction_t signTransaction;
  static test_init_t testInit;
  static commit_transaction_t commitTransaction;

  static std::string data_dir;

  /*test_genesis_xxx copy from rai::test_genesis_key, which be defined in rai/common.cpp*/
  static const char* test_genesis_prv;
  static const char* test_genesis_pub;
  static const char* test_genesis_account;
  static const char* test_prv2;
  static const char* test_pub2;
  static const char* test_account2;
};
void* OKWalletTest::wlt_mod = NULL;
get_address_t OKWalletTest::getAddress = nullptr;
produce_unsigned_tx_t OKWalletTest::produceUnsignedTx = nullptr;
sign_transaction_t OKWalletTest::signTransaction = nullptr;
test_init_t OKWalletTest::testInit = nullptr;
commit_transaction_t OKWalletTest::commitTransaction = nullptr;

std::string OKWalletTest::data_dir;

const char* OKWalletTest::test_genesis_prv = "34F0A37AAD20F4A260F0A5B3CB3D7FB50673212263E58A380BC10474BB039CE4";
const char* OKWalletTest::test_genesis_pub = "B0311EA55708D6A53C75CDBF88300259C6D018522FE3D4D0A242E431F9E8B6D0";
const char* OKWalletTest::test_genesis_account = "xrb_3e3j5tkog48pnny9dmfzj1r16pg8t1e76dz5tmac6iq689wyjfpiij4txtdo";
const char* OKWalletTest::test_prv2 = "78E5F97527656E723C86677DDD512CDD1EAAAC86250221D43E0FF554B4B06FB5";
const char* OKWalletTest::test_pub2 = "ADE7AB3EDC1023AFB3FF78F1D3CEA9E646D595249F1CB2CC610C8FCCA77939C0";
const char* OKWalletTest::test_account2 = "xrb_3dh9oezfr635oyszyy9jth9cmsk8tpckb9rwpd88456hskmqkgg1f9nmd4un";


TEST_F(OKWalletTest, getAddress) {
  std::string address;
  ASSERT_TRUE(getAddress(test_prv2, address));
  ASSERT_STREQ(test_account2, address.c_str());
}

TEST_F(OKWalletTest, produceUnsignedTx) {
  /*wallet id is AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA for compare*/
  const char* expected_utx = "{\n    \"action\": \"block_create\",\n    \"type\": \"state\",\n    \"wallet\": \"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\",\n    \"account\": \"xrb_3e3j5tkog48pnny9dmfzj1r16pg8t1e76dz5tmac6iq689wyjfpiij4txtdo\",\n    \"previous\": \"0\",\n    \"representative\": \"0\",\n    \"balance\": \"340282366920938463463374607431768201455\",\n    \"link\": \"xrb_3dh9oezfr635oyszyy9jth9cmsk8tpckb9rwpd88456hskmqkgg1f9nmd4un\"\n}\n";
  std::string utx;

  ASSERT_TRUE(produceUnsignedTx(test_genesis_account, test_account2, "10000", "testnet", data_dir.c_str(), utx));

  std::regex wallet_re("\"wallet\"\\s*:\\s*\"[0-9a-fA-F]{64}\"");
  const auto& rep_utx = std::regex_replace(utx, wallet_re, "\"wallet\": \"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\"");
  ASSERT_STREQ(expected_utx, rep_utx.c_str());
}

TEST_F(OKWalletTest, signTransaction) {
  /*
  std::vector<uint8_t> utx;

  ASSERT_TRUE(produceUnsignedTx(test_pair1.address, test_pair2.address, "100", "testnet", ".", utx));
  ASSERT_TRUE(utx.size() > 0);

  std::vector<uint8_t> stx;
  ASSERT_TRUE(signTransaction(utx, test_pair1.seed, "testnet", ".", stx));
  ASSERT_TRUE(stx.size() > utx.size());
  */
}

TEST_F(OKWalletTest, commitTransaction) {
  /*
  std::vector<uint8_t> utx;

  ASSERT_TRUE(produceUnsignedTx(test_pair1.address, test_pair2.address, "100", "testnet", ".", utx));
  ASSERT_TRUE(utx.size() > 0);

  std::vector<uint8_t> stx;
  ASSERT_TRUE(signTransaction(utx, test_pair1.seed, "testnet", ".", stx));
  ASSERT_TRUE(stx.size() > utx.size());

  std::string resstr;
  ASSERT_TRUE(commitTransaction(stx, "testnet", ".", resstr));
  */
}
