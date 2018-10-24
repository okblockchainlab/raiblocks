#include "wallet.h"
#include "rai/common.hpp"
#include "rai/rai_node/daemon.hpp"
#include "app_wrapper.h"
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <ed25519-donna/ed25519.h>


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
  const std::string& prvkey,
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
  request.put ("key", prvkey);

  {
		rai::raw_key prv;
		prv.data.clear ();
		boost::optional<std::string> key_text (request.get_optional<std::string> ("key"));
		if (key_text.is_initialized ())
		{
			auto error_key (prv.data.decode_hex (key_text.get ()));
			if (error_key)
			{
				//error_response (response, "Bad private key");
        return false;
			}
		}


		rai::uint256_union pub;
		ed25519_publickey (prv.data.bytes.data (), pub.bytes.data ());

		rai::uint256_union destination (0);
		boost::optional<std::string> destination_text (request.get_optional<std::string> ("destination"));
		if (destination_text.is_initialized ())
		{
			auto error_destination (destination.decode_account (destination_text.get ()));
			if (error_destination)
			{
				//error_response (response, "Bad destination account");
        return false;
			}
		}
		rai::block_hash source (0);
		boost::optional<std::string> source_text (request.get_optional<std::string> ("source"));
		if (source_text.is_initialized ())
		{
			auto error_source (source.decode_hex (source_text.get ()));
			if (error_source)
			{
				//error_response (response, "Invalid source hash");
        return false;
			}
		}

		rai::uint256_union previous (0);
		boost::optional<std::string> previous_text (request.get_optional<std::string> ("previous"));
		if (previous_text.is_initialized ())
		{
			auto error_previous (previous.decode_hex (previous_text.get ()));
			if (error_previous)
			{
				//error_response (response, "Invalid previous hash");
        return false;
			}
		}

		rai::uint256_union representative (0);
		boost::optional<std::string> representative_text (request.get_optional<std::string> ("representative"));
		if (representative_text.is_initialized ())
		{
			auto error_representative (representative.decode_account (representative_text.get ()));
			if (error_representative)
			{
				//error_response (response, "Bad representative account");
        return false;
			}
		}

		rai::uint128_union balance (0);
		boost::optional<std::string> balance_text (request.get_optional<std::string> ("balance"));
		if (balance_text.is_initialized ())
		{
			auto error_balance (balance.decode_dec (balance_text.get ()));
			if (error_balance)
			{
				//error_response (response, "Bad balance number");
        return false;
			}
		}

		rai::uint256_union link (0);
		boost::optional<std::string> link_text (request.get_optional<std::string> ("link"));
		if (link_text.is_initialized ())
		{
			auto error_link (link.decode_account (link_text.get ()));
			if (error_link)
			{
				auto error_link (link.decode_hex (link_text.get ()));
				if (error_link)
				{
					//error_response (response, "Bad link number");
          return false;
				}
			}
		}
		else
		{
			// Retrieve link from source or destination
			link = source.is_zero () ? destination : source;
		}

		uint64_t work (0);
		boost::optional<std::string> work_text (request.get_optional<std::string> ("work"));
		if (work_text.is_initialized ())
		{
			auto work_error (rai::from_string_hex (work_text.get (), work));
			if (work_error)
			{
				//error_response (response, "Bad work");
        return false;
			}
		}
		if (work == 0)
		{
			//work = aw.node()->work_generate_blocking (previous.is_zero () ? pub : previous);
		}

		rai::state_block state (pub, previous, representative, balance, link, prv, pub, work);
		//boost::property_tree::ptree response_l;
		//response_l.put ("hash", state.hash ().to_string ());
		std::string contents;
		state.serialize_json (contents);
		//response_l.put ("block", contents);
    stx = contents;
    return true;
  }

  /*
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
  */
}
