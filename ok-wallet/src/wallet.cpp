#include "wallet.h"
#include "rai/common.hpp"
#include "rai/rai_node/daemon.hpp"
#include "app_wrapper.h"
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>


bool _get_previous_block_hash(AppWrapper& aw, const std::string& account, std::string& previous)
{
  boost::property_tree::ptree request;
  request.put("action", "account_history");
  request.put("account", account);
  request.put("count", "1");

  boost::property_tree::ptree response;
  if (true != aw.send_rpc(request, response)) {
    return false;
  }

  previous = response.get("previous", "0");
  return true;
}

bool _get_balance(AppWrapper& aw, const std::string& account, rai::uint128_union& balance)
{
  boost::property_tree::ptree request;
  request.put("action", "account_balance");
  request.put("account", account);

  boost::property_tree::ptree response;
  if (true != aw.send_rpc(request, response)) {
    return false;
  }

  auto error_balance (balance.decode_dec(response.get<std::string>("balance")));
  if (error_balance) {
    return false;
  }
  return true;
}

bool _get_wallet_by_account(AppWrapper& aw, const std::string account, std::string& walletid)
{
  for (auto& wallet : aw.node()->wallets.items) {
    rai::transaction transaction(aw.node()->store.environment, nullptr, false);

    for (auto i(wallet.second->store.begin (transaction)), j(wallet.second->store.end ()); i != j; ++i) {
      if (account == rai::uint256_union (i->first.uint256 ()).to_account()) {
        walletid = wallet.first.to_string();
        return true;
      }
    }
  }

  return false;
}

bool _get_representative(AppWrapper& aw, const std::string& account, std::string& representative)
{
  boost::property_tree::ptree request;
  request.put("action", "account_representative");
  request.put("account", account);

  boost::property_tree::ptree response;
  if (true != aw.send_rpc(request, response)) {
    return false;
  }

  representative = response.get<std::string>("representative");
  return true;
}

bool GetAddressFromPrivateKey(const std::string& prv, std::string& address)
{
  rai::keypair kp(prv);
  address = kp.pub.to_account();
  return true;
}

bool produceUnsignedTx(const std::string& from, const std::string& to, const std::string& amount_s, const std::string& net_type, const char* data_dir, std::string& utx)
{
  AppWrapper aw(data_dir);

  rai::uint128_union amount (0);
  rai::uint128_union balance (0);
  if (true != _get_balance(aw, from, balance)) {
    return false;
  }
  if (amount.decode_dec(amount_s)) {
    return false;
  }
  if (balance.number() < amount.number()) {
    return false;
  }

  std::string walletid;
  if (true != _get_wallet_by_account(aw, from, walletid)) {
    return false;
  }

  std::string previous;
  if (true != _get_previous_block_hash(aw, from, previous)) {
    return false;
  }

  std::string representative;
  if (true != _get_representative(aw, from, representative)) {
    return false;
  }

  boost::property_tree::ptree request;
  request.put ("action", "block_create");
  request.put ("type", "state");
  request.put ("previous", previous);
  request.put ("representative", representative);
  request.put ("balance", balance.number() - amount.number());
  request.put ("link", to);

  std::stringstream ostream;
  boost::property_tree::write_json (ostream, request);
  utx = ostream.str();
  return true;
}

bool signTransaction(const std::string& utx, const std::string& prv, const std::string& net_type, const char* data_dir, std::string& stx)
{
  AppWrapper aw(data_dir);

  boost::property_tree::ptree request;
  std::stringstream istream (utx);
  boost::property_tree::read_json (istream, request);
  request.put ("key", prv);

  boost::property_tree::ptree response;
  if (true != aw.send_rpc(request, response)) {
    return false;
  }

  stx = response.get<std::string>("block", "");
  if (stx.empty()) {
    return false;
  } else {
    return true;
  }
}
