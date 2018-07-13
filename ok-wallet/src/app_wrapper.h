#pragma once

#include "rai/node/node.hpp"
#include "rai/node/rpc.hpp"
#include "rai/rai_node/daemon.hpp"
#include "boost/filesystem.hpp"


class AppWrapper
{
public:
  AppWrapper(boost::filesystem::path const & data_path);
  ~AppWrapper();

  std::shared_ptr<rai::node> node();

  bool send_rpc(boost::property_tree::ptree const & request, boost::property_tree::ptree& response);

  bool waitfor_catchup_ledger();

private:
  void _stop();
  void _poll();

private:
  std::shared_ptr<rai::node> mNode;
  std::unique_ptr<rai::rpc> mRpc;
  std::unique_ptr<rai::thread_runner> mRunner;

  boost::asio::io_service mService;
  rai::alarm mAlarm;
  rai::node_init mInit;
  rai_daemon::daemon_config mConfig;
  std::unique_ptr<rai::opencl_work> mOpencl;
  std::unique_ptr<rai::work_pool> mOpenclWork;

  bool pull_completed = false;
};
