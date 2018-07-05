#include "app_wrapper.h"
#include "rpc_response.h"

AppWrapper::AppWrapper(boost::filesystem::path const & data_path) :
  mAlarm(mService), mConfig(data_path)
{
  /*the code blow adjust from rai_daemon::daemon::run*/

	//boost::filesystem::create_directories (data_path);
	auto config_path ((data_path / "config.json"));
	std::fstream config_file;
	std::unique_ptr<rai::thread_runner> runner;
	auto error (rai::fetch_object (mConfig, config_path, config_file));
	if (error) {
		std::cerr << "Error deserializing config\n";
    return;
	}

	mConfig.node.logging.init (data_path);
	config_file.close ();
	auto opencl (rai::opencl_work::create (mConfig.opencl_enable, mConfig.opencl, mConfig.node.logging));
	rai::work_pool opencl_work (
    mConfig.node.work_threads,
    opencl ? [&opencl](rai::uint256_union const & root_a) { return opencl->generate_work (root_a); }
	         : std::function<boost::optional<uint64_t> (rai::uint256_union const &)> (nullptr)
  );

	try
	{
		auto node (std::make_shared<rai::node> (mInit, mService, data_path, mAlarm, mConfig.node, opencl_work));
		if (mInit.error ()) {
			std::cerr << "Error initializing node\n";
      return;
		}

		node->start ();
		std::unique_ptr<rai::rpc> rpc = get_rpc (mService, *node, mConfig.rpc);
		if (rpc && mConfig.rpc_enable) {
			rpc->start ();
		}

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

/* system::poll */
void AppWrapper::_poll()
{
	auto polled1 (mService.poll_one());
	if (polled1 == 0)
	{
		std::this_thread::sleep_for (std::chrono::milliseconds (50));
	}
}
