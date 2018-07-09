#include "test_helper.h"
#include "app_wrapper.h"
#include <stdexcept>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>


bool _init_config(std::string& data_dir)
{
  data_dir = (boost::filesystem::current_path() / "raiblock_testdata").string();

  if (boost::filesystem::exists(data_dir)) {
    throw std::runtime_error(std::string("data path '") + data_dir + "' had exist.");
  }

	boost::filesystem::create_directories (data_dir);
	rai_daemon::daemon_config config (data_dir);
  config.rpc_enable = true;
  config.rpc.enable_control = true;

	std::fstream config_file;
	boost::filesystem::path config_path(data_dir + "/config.json");
	if (false != rai::fetch_object (config, config_path, config_file)) {
    return false;
  }
	config.node.logging.init (data_dir);
	config_file.close ();

  return true;
}

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

bool testInit(const std::string& test_prv1, const std::string& test_prv2, std::string& data_dir)
{
  if (true != _init_config(data_dir)) {
    return false;
  }

  AppWrapper aw(data_dir);

  std::string walletid;
  if (true != _wallet_create(aw, walletid)) {
    return false;
  }

  std::string test_account;
  if (true != _wallet_add(aw, walletid, test_prv1, test_account)) {
    return false;
  }
  if (true != _wallet_add(aw, walletid, test_prv2, test_account)) {
    return false;
  }

  return true;
}
