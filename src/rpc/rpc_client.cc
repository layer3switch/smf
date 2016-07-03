#include "rpc/rpc_client.h"
// seastar
#include <core/reactor.hh>
#include <net/api.hh>
// smf
#include "log.h"
#include "rpc/rpc_envelope_utils.h"
namespace smf {

rpc_client::rpc_client(ipv4_addr server_addr)
  : server_addr_(std::move(server_addr)) {
  LOG_INFO("constructed rpc_client");
}


future<> rpc_client::recv(rpc_recv_context &&ctx) {
  LOG_INFO("HOORAY GOT A REPLY");
  return make_ready_future<>();
}

future<> rpc_client::send(rpc_envelope &&req, bool oneway) {
  return connect().then([this, r = std::move(req), oneway]() mutable {
    return chain_send(std::move(r), oneway);
  });
}
future<> rpc_client::stop() { return make_ready_future(); }
rpc_client::~rpc_client() {
  if(conn_) {
    delete conn_;
  }
}

future<> rpc_client::connect() {
  LOG_INFO("connecting");
  if(conn_ != nullptr) {
    LOG_INFO("socket non null");
    return make_ready_future<>();
  }
  LOG_INFO("Not connected, creating new tcp connection");
  socket_address local =
    socket_address(::sockaddr_in{AF_INET, INADDR_ANY, {0}});
  return engine()
    .net()
    .connect(make_ipv4_address(server_addr_), local, seastar::transport::TCP)
    .then([this](connected_socket fd) mutable {
      LOG_INFO("creating a new rpc_client_connection");
      conn_ = new rpc_client_connection(std::move(fd));
      LOG_INFO("finished creating rpc client connection");
    });
}
future<> rpc_client::chain_send(rpc_envelope &&req, bool oneway) {
  LOG_INFO("chain_send");
  return rpc_envelope_send(conn_->ostream(), std::move(req))
    .then([this, oneway] { return chain_recv(oneway); });
}
future<> rpc_client::chain_recv(bool oneway) {
  LOG_INFO("chain_recv");
  if(oneway) {
    return make_ready_future<>();
  }
  return rpc_recv_context::parse(conn_->istream())
    .then([this](std::experimental::optional<rpc_recv_context> ctx) mutable {
      if(!ctx) {
        LOG_ERROR("Invalid response");
        return make_ready_future<>();
      }
      return recv(std::move(ctx.value()));
    });
}
} // namespace