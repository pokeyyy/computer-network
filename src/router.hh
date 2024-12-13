#pragma once

#include <memory>
#include <optional>
#include <set>

#include "exception.hh"
#include "network_interface.hh"
using namespace std;

class router_item {
public:
  uint32_t route_prefix;
  uint8_t prefix_length;
  optional<Address> next_hop;
  size_t interface_num;
  router_item(uint32_t rp, uint8_t pl, std::optional<Address> nh, size_t in)
        : route_prefix(rp), prefix_length(pl), next_hop(nh), interface_num(in) {}
  bool operator<(const router_item& other) const {
    return prefix_length > other.prefix_length; // 从大到小排序
  }
};
// \brief A router that has multiple network interfaces and
// performs longest-prefix-match routing between them.
class Router
{
public:
  // Add an interface to the router
  // \param[in] interface an already-constructed network interface
  // \returns The index of the interface after it has been added to the router
  size_t add_interface( std::shared_ptr<NetworkInterface> interface )
  {
    _interfaces.push_back( notnull( "add_interface", std::move( interface ) ) );
    return _interfaces.size() - 1;
  }

  // Access an interface by index
  std::shared_ptr<NetworkInterface> interface( const size_t N ) { return _interfaces.at( N ); }

  // Add a route (a forwarding rule)
  void add_route( uint32_t route_prefix,
                  uint8_t prefix_length,
                  std::optional<Address> next_hop,
                  size_t interface_num );

  // Route packets between the interfaces
  void route();

private:
  // The router's collection of network interfaces
  std::vector<std::shared_ptr<NetworkInterface>> _interfaces {};
  set<router_item> routing_table_ {};
};
