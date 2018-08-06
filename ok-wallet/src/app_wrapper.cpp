#include "app_wrapper.h"
#include "rpc_response.h"


AppWrapper::AppWrapper() :
  mAlarm(mService), mConfig("")
{
  /*the code blow adjust from rai_daemon::daemon::run*/

  mCfgDir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
  if (true != _init_config(mCfgDir)) {
    std::cerr << "init config failed in path: " << mCfgDir << std::endl;
    return;
  }

  rai_daemon::daemon_config config(mCfgDir);

  auto config_path ((mCfgDir / "config.json"));
  std::fstream config_file;
  auto error (rai::fetch_object (config, config_path, config_file));
  if (error) {
    std::cerr << "Error deserializing config\n";
    return;
  }

  config.node.logging.init (mCfgDir);
  config_file.close ();
  mOpencl = rai::opencl_work::create (config.opencl_enable, config.opencl, config.node.logging);
  auto opencl = mOpencl.get();
  mOpenclWork = std::make_unique<rai::work_pool> (
    config.node.work_threads,
    mOpencl ? [opencl](rai::uint256_union const & root_a) { return opencl->generate_work (root_a); }
      : std::function<boost::optional<uint64_t> (rai::uint256_union const &)> (nullptr)
  );

  try
  {
    auto node (std::make_shared<rai::node> (mInit, mService, mCfgDir, mAlarm, config.node, *(mOpenclWork.get())));
    if (mInit.error ()) {
      std::cerr << "Error initializing node\n";
      return;
    }

    node->bootstrap_initiator.add_result_observer([this](bool completed){
      pull_completed = completed;
    });
    node->start ();
    std::unique_ptr<rai::rpc> rpc = get_rpc (mService, *node, config.rpc);
    if (rpc && config.rpc_enable) {
      rpc->start ();
    }

    mConfig = std::move(config);
    mRunner = std::make_unique<rai::thread_runner> (mService, node->config.io_threads);
    mRpc = std::move(rpc);
    mNode = std::move(node);
  }
  catch (const std::runtime_error & e)
  {
    std::cerr << "Error while running node (" << e.what () << ")\n";
  }
}

AppWrapper::~AppWrapper()
{
  _stop();
  boost::filesystem::remove_all(mCfgDir);
}

std::shared_ptr<rai::node> AppWrapper::node()
{
  return mNode;
}

void AppWrapper::_stop()
{
  if (mRpc) {
    mRpc->stop();
  }
  if (mNode) {
    mNode->stop();
  }

  mRunner.reset();
  mRpc.reset();
  mNode.reset();
}

bool AppWrapper::send_rpc(boost::property_tree::ptree const & request, boost::property_tree::ptree& response)
{
  if (nullptr == mRpc) {
    return false;
  }

  RpcResponse rr(request, mRpc.get(), mService);
  while (rr.status == 0) {
    _poll();
  }
  if (200 != rr.status) {
    return false;
  }

  response = rr.json;
  return true;
}

bool AppWrapper::waitfor_catchup_ledger()
{
  int try_cnt = 0;
  int waiting_cnt = 0;

  while(try_cnt < 4) {
    waiting_cnt = 0;

    while(node()->bootstrap_initiator.in_progress()) {
      auto attempt = node()->bootstrap_initiator.current_attempt();
      assert(nullptr != attempt);
      if (attempt->waiting()) {
        waiting_cnt++;
      } else {
        waiting_cnt = 0;
      }

      //break when wait for pull more then 10 times continuously
      if (waiting_cnt > 10) {
        break;
      }

      sleep(1);
    }

    if (true != pull_completed) {
      sleep(5);//rai::node::ongoing_bootstrap will sleep 5s for the first 3 times.
      try_cnt++;
      continue;
    }
  }

  return pull_completed;
}

/* system::poll */
void AppWrapper::_poll()
{
  auto polled1 (mService.poll_one());
  if (polled1 == 0)
  {
    std::this_thread::sleep_for (std::chrono::milliseconds (50));
  }
}

bool AppWrapper::_init_config(const boost::filesystem::path& data_dir)
{
  if (!boost::filesystem::exists(data_dir)) {
    boost::filesystem::create_directories (data_dir);
  }

  rai_daemon::daemon_config config (data_dir);
  config.rpc_enable = true;
  config.rpc.enable_control = true;

  std::fstream config_file;
  boost::filesystem::path config_path((data_dir / "config.json"));
  if (false != rai::fetch_object (config, config_path, config_file)) {
    return false;
  }
  config.node.logging.init (data_dir);
  config_file.close ();

  return true;
}
