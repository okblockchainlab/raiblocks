#include "gtest/gtest.h"
#include "wallet.h"
#include <dlfcn.h>

struct key_pair_t{
  const char* prv;
  const char* pub;
  const char* address; //account
};

key_pair_t test_kp1 = {
  "C80CF9F756CF4338F14062F9E974E25EA02EFA293CDD5A7B1BAACAB0660E40D6",
  "C883125EFEA3B5072C8BF9AD0B0F8BCDC39DEC1E1FA7C642B2407F448E82870E",
  "xrb_3k654bhhxaxo1wpaqyff3e9rqmg5mqp3w9x9rs3d6i5zak9a73rgwauuqocw"
};
key_pair_t test_kp2 = {
  "78E5F97527656E723C86677DDD512CDD1EAAAC86250221D43E0FF554B4B06FB5",
  "ADE7AB3EDC1023AFB3FF78F1D3CEA9E646D595249F1CB2CC610C8FCCA77939C0",
  "xrb_3dh9oezfr635oyszyy9jth9cmsk8tpckb9rwpd88456hskmqkgg1f9nmd4un"
};

typedef bool (*get_address_t)(const std::string& prv, std::string& address);
typedef bool (*produce_unsigned_tx_t)(const std::string& from, const std::string& to, const std::string& amount, const std::string& net_type, const char* data_dir, std::string& utx);
typedef bool (*sign_transaction_t)(const std::vector<uint8_t>& utx, const std::string& seed, const std::string& net_type, const char* data_dir, std::vector<uint8_t>& stx);
typedef bool (*commit_transaction_t)(const std::vector<uint8_t>& stx, const std::string& net_type, const char* data_dir, std::string& result_str);

class OKWalletTest : public ::testing::Test {
public:
  static void SetUpTestCase() {
    //FIXME: use a better path
    wlt_mod = dlopen("./libraiblocks.dylib", RTLD_LAZY);
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
  std::string address;
  ASSERT_TRUE(getAddress(test_kp1.prv, address));
  ASSERT_STREQ(test_kp1.address, address.c_str());

  ASSERT_TRUE(getAddress(test_kp2.prv, address));
  ASSERT_STREQ(test_kp2.address, address.c_str());
}

TEST_F(OKWalletTest, produceUnsignedTx) {
  //std::vector<uint8_t> expect_tx = {1, 2, 3};
  std::string utx;

  ASSERT_TRUE(produceUnsignedTx(test_kp1.address, test_kp2.address, "100", "testnet", "/Users/oker/Library/RaiBlocksTest", utx));
  ASSERT_TRUE(utx.length() > 0);
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
