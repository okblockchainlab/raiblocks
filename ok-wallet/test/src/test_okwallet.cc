#include "gtest/gtest.h"
#include "wallet.h"
#include <dlfcn.h>


typedef bool (*get_address_t)(const std::string& seed, std::string& address);
typedef bool (*produce_unsigned_tx_t)(const std::string& from, const std::string& to, const std::string& amount, const std::string& net_type, const char* data_dir, std::vector<uint8_t>& utx);
typedef bool (*sign_transaction_t)(const std::vector<uint8_t>& utx, const std::string& seed, const std::string& net_type, const char* data_dir, std::vector<uint8_t>& stx);
typedef bool (*commit_transaction_t)(const std::vector<uint8_t>& stx, const std::string& net_type, const char* data_dir, std::string& result_str);

class OKWalletTest : public ::testing::Test {
public:
  static void SetUpTestCase() {
    //FIXME: use a better path
    wlt_mod = dlopen("./libstellar-core.dylib", RTLD_LAZY);
    getAddress = (get_address_t)dlsym(wlt_mod, "GetAddressFromPrivateKey");
    produceUnsignedTx = (produce_unsigned_tx_t)dlsym(wlt_mod, "produceUnsignedTx");
    signTransaction = (sign_transaction_t)dlsym(wlt_mod, "signTransaction");
    commitTransaction = (commit_transaction_t)dlsym(wlt_mod, "commitTransaction");
  }
  static void TearDownTestCase() {
    if (NULL != wlt_mod) {
      dlclose(wlt_mod);
      wlt_mod = NULL;
    }
  }

  virtual void SetUp() {
  }

  virtual void TearDown() {
  }

  static void* wlt_mod;
  static get_address_t getAddress;
  static produce_unsigned_tx_t produceUnsignedTx;
  static sign_transaction_t signTransaction;
  static commit_transaction_t commitTransaction;
};
void* OKWalletTest::wlt_mod = NULL;
get_address_t OKWalletTest::getAddress = nullptr;
produce_unsigned_tx_t OKWalletTest::produceUnsignedTx = nullptr;
sign_transaction_t OKWalletTest::signTransaction = nullptr;
commit_transaction_t OKWalletTest::commitTransaction = nullptr;

TEST_F(OKWalletTest, getAddress) {
  /*
  std::string address;
  ASSERT_TRUE(getAddress(test_pair1.seed, address));

  ASSERT_STREQ(test_pair1.address, address.c_str());
  */
}

TEST_F(OKWalletTest, produceUnsignedTx) {
  /*
  std::vector<uint8_t> expect_tx = {1, 2, 3};
  std::vector<uint8_t> utx;

  ASSERT_TRUE(produceUnsignedTx(test_pair1.address, test_pair2.address, "100", "testnet", ".", utx));
  ASSERT_TRUE(utx.size() > 0);
  */
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
