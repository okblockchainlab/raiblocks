#include "test_helper.h"
#include "app_wrapper.h"
#include <stdexcept>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>


bool _wallet_create(AppWrapper& aw, std::string& walletid)
{
	boost::property_tree::ptree request;
	request.put("action", "wallet_create");

  boost::property_tree::ptree response;
  if (true != aw.send_rpc(request, response)) {
    return false;
  }

  walletid = response.get<std::string>("wallet");
  return true;
}

bool _wallet_add(AppWrapper& aw, const std::string& walletid, const std::string& prv, std::string& account)
{
	boost::property_tree::ptree request;
	request.put("action", "wallet_add");
  request.put("wallet", walletid);
  request.put("key", prv);
  request.put("work", "false");

  boost::property_tree::ptree response;
  if (true != aw.send_rpc(request, response)) {
    return false;
  }

  account = response.get<std::string>("account");
  return true;
}

bool _wallet_add_watch(AppWrapper& aw, const std::string& walletid, const std::vector<std::string>& accounts)
{
  boost::property_tree::ptree request;

  request.put("action", "wallet_add_watch");
  request.put("wallet", walletid);
  boost::property_tree::ptree accounts_subtree;
  for (const auto& account : accounts) {
    boost::property_tree::ptree entry;
    entry.put ("", account);
    accounts_subtree.push_back(std::make_pair ("", entry));
  }
  request.add_child ("accounts", accounts_subtree);

  boost::property_tree::ptree response;
  if (true != aw.send_rpc(request, response)) {
    return false;
  }

  if(response.get<std::string>("success", "false") == "false") {
    return false;
  } else {
    return true;
  }
}

bool _set_representative(AppWrapper& aw, const std::string& walletid, const std::string& account, const std::string& representative)
{
  boost::property_tree::ptree request;
  request.put("action", "account_representative_set");
  request.put("wallet", walletid);
  request.put("account", account);
  request.put("representative", representative);

  boost::property_tree::ptree response;
  if (true != aw.send_rpc(request, response)) {
    return false;
  }

  if (response.get("block", "0") != "0") {
    return true;
  } else {
    return false;
  }
}

bool testInit(const std::string& test_prv1, const std::string& test_account2)
{
  if (rai::rai_network != rai::rai_networks::rai_test_network) {
    std::cerr << "You can not call 'testInit' when this module is not compiled with test network." << std::endl;
    abort();
  }

  AppWrapper aw;

  std::string walletid;
  if (true != _wallet_create(aw, walletid)) {
    return false;
  }

  /*钱包里本来可以完全不需要私钥，只需账号然后调用_wallet_add_watch即可。
    但raiblocks规定每个账号链的第一个transaction必须是一个open transaction。这有两种办法
    实现，一种简单的调用_wallet_add直接添加一个私钥； 另一种方法利用RPC协议中的block_create
    方法创建一个open block，然后发送这个open block。这里为了简单使用的第一种方法，因而也就需要一个私钥。*/
  std::string test_account1;
  if (true != _wallet_add(aw, walletid, test_prv1, test_account1)) {
    return false;
  }

  if (true != _wallet_add_watch(aw, walletid, {test_account2})) {
    return false;
  }

  if (true != _set_representative(aw, walletid, test_account1, test_account1)) {
    return false;
  }
  if (true != _set_representative(aw, walletid, test_account2, test_account2)) {
    return false;
  }


  return true;
}

bool commitTransaction(const std::string& stx, const std::string& net_type, std::string& result_str)
{
  AppWrapper aw;

  boost::property_tree::ptree request;
  request.put("action", "process");
  request.put("block", stx);
  request.put("force", true);

  boost::property_tree::ptree response;
  if (true != aw.send_rpc(request, response)) {
    return false;
  }

  if (response.get<std::string>("hash", "0") == "0") {
    return false;
  }

  std::stringstream ostream;
  boost::property_tree::write_json (ostream, response);
  result_str = ostream.str();
  return true;
}
