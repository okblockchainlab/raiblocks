#include "wallet.h"
#include "rai/common.hpp"
#include "rai/rai_node/daemon.hpp"
#include "app_wrapper.h"
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>


  /*
{
  Request:

  {
    "action": "account_history",
    "account": "xrb_1ipx847tk8o46pwxt5qjdbncjqcbwcc1rrmqnkztrfjy5k7z4imsrata9est",
    "count": "1"
  }
  Response:

  {
      "account": "xrb_1ipx847tk8o46pwxt5qjdbncjqcbwcc1rrmqnkztrfjy5k7z4imsrata9est",
      "history": [
          {
              "type": "send",
              "account": "xrb_38ztgpejb7yrm7rr586nenkn597s3a1sqiy3m3uyqjicht7kzuhnihdk6zpz",
              "amount": "80000000000000000000000000000000000",
              "hash": "80392607E85E73CC3E94B4126F24488EBDFEB174944B890C97E8F36D89591DC5"
          }
      ],
      "previous": "8D3AB98B301224253750D448B4BD997132400CEDD0A8432F775724F2D9821C72"
  }
  如果一个block也没有，则没有previous字段
}
  */
std::string _get_previous_block_hash(AppWrapper& aw, const std::string& account)
{
	boost::property_tree::ptree request;
	request.put("action", "account_history");
  request.put("account", account);
  request.put("count", "1");

  boost::property_tree::ptree response;
  if (true != aw.send_rpc(request, response)) {
    return "";
  }

  return response.get("previous", "0"); //or "0000000000000000000000000000000000000000000000000000000000000000" ?
}

std::string _get_balance(AppWrapper& aw, const std::string& account)
{
	boost::property_tree::ptree request;
	request.put("action", "account_balance");
  request.put("account", account);

  boost::property_tree::ptree response;
  if (true != aw.send_rpc(request, response)) {
    return "";
  }

  return response.get<std::string>("balance");
}


bool GetAddressFromPrivateKey(const std::string& prv, std::string& address)
{
  rai::keypair kp(prv);
  address = kp.pub.to_account();
  return true;
}

bool produceUnsignedTx(const std::string& from, const std::string& to, const std::string& amount, const std::string& net_type, const char* data_dir, std::string& utx)
{
  AppWrapper aw(data_dir);

  const auto balance = _get_balance(aw, from);
  if (balance.empty()) {
    return false;
  }

  const auto balance_amount = stoull(balance);
  const auto send_amount = stoull(amount);
  if (balance_amount < send_amount) {
    return false;
  }

  const auto& previous = _get_previous_block_hash(aw, from);
  if (previous.empty()) {
    return false;
  }

	boost::property_tree::ptree request;
	request.put ("action", "block_create");
	request.put ("type", "state");
	//request.put ("wallet", system.nodes[0]->wallets.items.begin ()->first.to_string ());
	request.put ("account", from);
	request.put ("previous", previous);
	//request.put ("representative", rai::test_genesis_key.pub.to_account ());
	request.put ("balance", std::to_string(balance_amount - send_amount));
	request.put ("link", to);

	std::stringstream ostream;
	boost::property_tree::write_json (ostream, request);
  utx = ostream.str();
  return true;
}
