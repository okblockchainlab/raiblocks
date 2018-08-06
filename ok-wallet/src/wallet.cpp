#include "wallet.h"
#include "rai/common.hpp"
#include "rai/rai_node/daemon.hpp"
#include "app_wrapper.h"
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>


bool GetAddressFromPrivateKey(const std::string& prv, std::string& address)
{
  rai::keypair kp(prv);
  address = kp.pub.to_account();
  return true;
}

bool produceUnsignedTx(
  const std::string& link,
  const std::string& previous,
  const std::string& representative,
  const std::string& balance_s,
  std::string& utx)
{
  rai::uint128_union balance (0);
  if (balance.decode_dec(balance_s)) {
    return false;
  }

  boost::property_tree::ptree request;
  request.put ("action", "block_create");
  request.put ("type", "state");
  request.put ("previous", previous);
  request.put ("representative", representative);
  request.put ("balance", balance.number());
  request.put ("link", link);

  std::stringstream ostream;
  boost::property_tree::write_json (ostream, request);
  utx = ostream.str();
  return true;
}

bool signTransaction(
  const std::string& utx,
  const std::string& prv,
  const std::string& net_type,
  std::string& stx)
{
  if (("main" == net_type && rai::rai_networks::rai_live_network != rai::rai_network) ||
      ("test" == net_type && rai::rai_networks::rai_test_network != rai::rai_network) ||
      ("beta" == net_type && rai::rai_networks::rai_beta_network != rai::rai_network) ) {
      throw "you have mismatch net type on running and compiling.";
      return false;
  }

  AppWrapper aw;

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
