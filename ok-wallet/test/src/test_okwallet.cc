#include "gtest/gtest.h"
#include "wallet.h"
#include <dlfcn.h>
#include <regex>
#include <unistd.h>
#include <stdlib.h>


typedef bool (*get_address_t)(const std::string& prv, std::string& address);
typedef bool (*produce_unsigned_tx_t)(const std::string& from, const std::string& to, const std::string& amount, const std::string& net_type, const char* data_dir, std::string& utx);
typedef bool (*sign_transaction_t)(const std::string& utx, const std::string& seed, const std::string& net_type, const char* data_dir, std::string& stx);
typedef bool (*test_init_t)(const std::string& test_prv1, const std::string& test_account2, const std::string& data_dir);
typedef bool (*commit_transaction_t)(const std::string& stx, const std::string& net_type, const char* data_dir, std::string& result_str);

class OKWalletTest : public ::testing::Test {
public:
  static void SetUpTestCase() {
    const auto& cwd = _get_current_dir();
    ASSERT_FALSE(cwd.empty());

    data_dir = cwd + "/raiblock_testdata";

#ifdef __APPLE__
    const auto& mod_path = cwd + "/libraiblocks.dylib";
#elif __linux__
    const auto& mod_path = cwd + "/libraiblocks.so";
#endif

    wlt_mod = dlopen(mod_path.c_str(), RTLD_LAZY);
    getAddress = (get_address_t)dlsym(wlt_mod, "GetAddressFromPrivateKey");
    produceUnsignedTx = (produce_unsigned_tx_t)dlsym(wlt_mod, "produceUnsignedTx");
    signTransaction = (sign_transaction_t)dlsym(wlt_mod, "signTransaction");

    testInit = (test_init_t)dlsym(wlt_mod, "testInit");
    commitTransaction = (commit_transaction_t)dlsym(wlt_mod, "commitTransaction");

    ASSERT_TRUE(testInit(test_genesis_prv, test_account2, data_dir));
    data_dir_created_by_myself = true;
  }

  static void TearDownTestCase() {
    if (NULL != wlt_mod) {
      dlclose(wlt_mod);
      wlt_mod = NULL;
    }

    if (data_dir_created_by_myself) {
      system(("rm -rf " + data_dir).c_str());
    }
  }

  static std::string _get_current_dir()
  {
    auto* buf = getcwd(NULL, 0);
    if (nullptr == buf) {
      return "";
    }

    std::string res(buf);
    free(buf);
    return res;
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
  static bool data_dir_created_by_myself;

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
bool OKWalletTest::data_dir_created_by_myself = false;

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
  const char* expected_utx = "{\n    \"action\": \"block_create\",\n    \"type\": \"state\",\n    \"previous\": \"04270D7F11C4B2B472F2854C5A59F2A7E84226CE9ED799DE75744BD7D85FC9D9\",\n    \"representative\": \"xrb_3e3j5tkog48pnny9dmfzj1r16pg8t1e76dz5tmac6iq689wyjfpiij4txtdo\",\n    \"balance\": \"340282366920938463463374607431768201455\",\n    \"link\": \"xrb_3dh9oezfr635oyszyy9jth9cmsk8tpckb9rwpd88456hskmqkgg1f9nmd4un\"\n}\n";
  std::string utx;

  ASSERT_TRUE(produceUnsignedTx(test_genesis_account, test_account2, "10000", "testnet", data_dir.c_str(), utx));

  ASSERT_STREQ(expected_utx, utx.c_str());
}

TEST_F(OKWalletTest, signTransaction) {
  /*replace work  with "AAAAAAAAAAAAAAAA" for compare*/
  std::string expect_stx = "{\n    \"type\": \"state\",\n    \"account\": \"xrb_3e3j5tkog48pnny9dmfzj1r16pg8t1e76dz5tmac6iq689wyjfpiij4txtdo\",\n    \"previous\": \"04270D7F11C4B2B472F2854C5A59F2A7E84226CE9ED799DE75744BD7D85FC9D9\",\n    \"representative\": \"xrb_3e3j5tkog48pnny9dmfzj1r16pg8t1e76dz5tmac6iq689wyjfpiij4txtdo\",\n    \"balance\": \"340282366920938463463374607431768201455\",\n    \"link\": \"ADE7AB3EDC1023AFB3FF78F1D3CEA9E646D595249F1CB2CC610C8FCCA77939C0\",\n    \"link_as_account\": \"xrb_3dh9oezfr635oyszyy9jth9cmsk8tpckb9rwpd88456hskmqkgg1f9nmd4un\",\n    \"signature\": \"0A6E402D42FC01D1FBBA9DD4B8BC509A6D6AF257EDCA0A67F95303C461FD6ECD2819DDA810DFEC6B0EF383BBBBC0500A83E1E8E40FCD18987EDEE8237748AC08\",\n    \"work\": \"AAAAAAAAAAAAAAAA\"\n}\n";
  std::string utx;

  ASSERT_TRUE(produceUnsignedTx(test_genesis_account, test_account2, "10000", "testnet", data_dir.c_str(), utx));
  ASSERT_FALSE(utx.empty());

  std::string stx;
  ASSERT_TRUE(signTransaction(utx, test_genesis_prv, "testnet", data_dir.c_str(), stx));

  std::regex stx_re("\\\"work\\\"\\s*:\\s*\\\"[0-9a-fA-F]{16}\\\"");
  const auto& rep_stx = std::regex_replace(stx, stx_re, "\"work\": \"AAAAAAAAAAAAAAAA\"");
  ASSERT_STREQ(expect_stx.c_str(), rep_stx.c_str());
}

TEST_F(OKWalletTest, commitTransaction) {
  const char* exptect_commit_res = "{\n    \"hash\": \"46F35CC292EA68103BF9BE81C87D3F3A9904F27ECB5A93EB158626D939529910\"\n}\n";
  std::string utx;

  ASSERT_TRUE(produceUnsignedTx(test_genesis_account, test_account2, "10000", "testnet", data_dir.c_str(), utx));
  ASSERT_FALSE(utx.empty());

  std::string stx;
  ASSERT_TRUE(signTransaction(utx, test_genesis_prv, "testnet", data_dir.c_str(), stx));
  ASSERT_FALSE(stx.empty());

  std::string resstr;
  ASSERT_TRUE(commitTransaction(stx, "testnet", data_dir.c_str(), resstr));
  ASSERT_STREQ(exptect_commit_res, resstr.c_str());
}
