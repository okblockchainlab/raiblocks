#pragma once

class RpcResponse
{
public:
  RpcResponse (boost::property_tree::ptree const & request_a, rai::rpc* rpc_a, boost::asio::io_service & service_a) :
    request (request_a),
    sock (service_a),
    status (0)
  {
    sock.async_connect (rai::tcp_endpoint (rpc_a->config.address, rpc_a->config.port), [this](boost::system::error_code const & ec) {
      if (ec) {
        status = 400;
        return;
      }

      std::stringstream ostream;
      boost::property_tree::write_json (ostream, request);
      req.method (boost::beast::http::verb::post);
      req.target ("/");
      req.version (11);
      ostream.flush ();
      req.body () = ostream.str ();
      req.prepare_payload ();
      boost::beast::http::async_write (sock, req, [this](boost::system::error_code const & ec, size_t bytes_transferred) {
        if (ec) {
          status = 600;
          return;
        }

        boost::beast::http::async_read (sock, sb, resp, [this](boost::system::error_code const & ec, size_t bytes_transferred) {
          if (ec) {
            status = 400;
            return;
          }

          std::stringstream body (resp.body ());
          try
          {
            boost::property_tree::read_json (body, json);
            status = 200;
          }
          catch (std::exception & e)
          {
            status = 500;
          }
        });
      });
    });
  }

  boost::property_tree::ptree const & request;
  boost::asio::ip::tcp::socket sock;
  boost::property_tree::ptree json;
  boost::beast::flat_buffer sb;
  boost::beast::http::request<boost::beast::http::string_body> req;
  boost::beast::http::response<boost::beast::http::string_body> resp;
  int status;
};
