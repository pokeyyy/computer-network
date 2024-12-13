#include "router.hh"

#include <iostream>
#include <limits>

using namespace std;

// route_prefix: The "up-to-32-bit" IPv4 address prefix to match the datagram's destination address against
// prefix_length: For this route to be applicable, how many high-order (most-significant) bits of
//    the route_prefix will need to match the corresponding bits of the datagram's destination address?
// next_hop: The IP address of the next hop. Will be empty if the network is directly attached to the router (in
//    which case, the next hop address should be the datagram's final destination).
// interface_num: The index of the interface to send the datagram out on.
void Router::add_route( const uint32_t route_prefix,
                        const uint8_t prefix_length,
                        const optional<Address> next_hop,
                        const size_t interface_num )
{
  cerr << "DEBUG: adding route " << Address::from_ipv4_numeric( route_prefix ).ip() << "/"
       << static_cast<int>( prefix_length ) << " => " << ( next_hop.has_value() ? next_hop->ip() : "(direct)" )
       << " on interface " << interface_num << "\n";

  router_table_.insert({route_prefix,prefix_length,next_hop,interface_num});
}

// Go through all the interfaces, and route every incoming datagram to its proper outgoing interface.
void Router::route()
{
  for (auto& interfacePtr : _interfaces) {
    auto& datagram_all = interfacePtr->datagrams_received();
    while(!datagram_all.empty()){
      InternetDatagram datagram = datagram_all.front();
      datagram_all.pop();
      uint32_t dst = datagram.header.dst;

      auto router_it = router_table_.begin();
      for(; router_it != router_table_.end(); ++ router_it){
        uint32_t mask = ~((1 << (32 - router_it->prefix_length)) - 1);
        if((router_it->route_prefix & mask) == (dst & mask))
          break;
      }
      if(router_it == router_table_.end())
        continue;
      
      datagram.header.ttl--;
      if(datagram.header.ttl == 0)
        continue;
      datagram.header.compute_checksum();

      Address next_addr = router_it->next_hop.has_value()? router_it->next_hop.value() : Address::from_ipv4_numeric(dst);
      interface(router_it->interface_num)->send_datagram(datagram, next_addr);
    }
  }
}
