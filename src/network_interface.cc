#include <iostream>

#include "arp_message.hh"
#include "exception.hh"
#include "network_interface.hh"

using namespace std;

//! \param[in] ethernet_address Ethernet (what ARP calls "hardware") address of the interface
//! \param[in] ip_address IP (what ARP calls "protocol") address of the interface
NetworkInterface::NetworkInterface( string_view name,
                                    shared_ptr<OutputPort> port,
                                    const EthernetAddress& ethernet_address,
                                    const Address& ip_address )
  : name_( name )
  , port_( notnull( "OutputPort", move( port ) ) )
  , ethernet_address_( ethernet_address )
  , ip_address_( ip_address )
{
  cerr << "DEBUG: Network interface has Ethernet address " << to_string( ethernet_address ) << " and IP address "
       << ip_address.ip() << "\n";
}

//! \param[in] dgram the IPv4 datagram to be sent
//! \param[in] next_hop the IP address of the interface to send it to (typically a router or default gateway, but
//! may also be another host if directly connected to the same network as the destination) Note: the Address type
//! can be converted to a uint32_t (raw 32-bit IP address) by using the Address::ipv4_numeric() method.
void NetworkInterface::send_datagram( const InternetDatagram& dgram, const Address& next_hop )
{
  uint32_t next_address = next_hop.ipv4_numeric();
  auto it = arp_table_.find( next_address );
  if ( it == arp_table_.end() ) {
    if ( arp_5_.find( next_address ) == arp_5_.end() ) {
      EthernetAddress target_eth = EthernetAddress {};
      ARPMessage arp { .opcode = ARPMessage::OPCODE_REQUEST,
                       .sender_ethernet_address = ethernet_address_,
                       .sender_ip_address = ip_address_.ipv4_numeric(),
                       .target_ethernet_address = target_eth,
                       .target_ip_address = next_address };

      EthernetAddress dst = ETHERNET_BROADCAST;
      EthernetAddress src = ethernet_address_;
      EthernetHeader header { .dst = dst, .src = src, .type = EthernetHeader::TYPE_ARP };
      EthernetFrame frame { .header = header, .payload = serialize( arp ) };
      transmit( frame );
      arp_5_.emplace( next_address, 5000 );
    }
    arp_ip_waiting_.emplace( next_address, dgram );
  } else {
    EthernetAddress dst = it->second.eth_addr;
    EthernetAddress src = ethernet_address_;
    EthernetHeader header { .dst = dst, .src = src, .type = EthernetHeader::TYPE_IPv4 };
    EthernetFrame frame { .header = header, .payload = serialize( dgram ) };
    transmit( frame );
  }
}

//! \param[in] frame the incoming Ethernet frame
void NetworkInterface::recv_frame( const EthernetFrame& frame )
{
  if ( frame.header.dst != ETHERNET_BROADCAST && frame.header.dst != ethernet_address_ )
    return;

  if ( frame.header.type == EthernetHeader::TYPE_IPv4 ) {
    InternetDatagram datagram;
    if ( !parse( datagram, frame.payload ) )
      return;
    datagrams_received_.emplace( datagram );
  } else if ( frame.header.type == EthernetHeader::TYPE_ARP ) {
    ARPMessage arp_msg;
    if ( !parse( arp_msg, frame.payload ) )
      return;
    uint32_t src_ip = arp_msg.sender_ip_address;
    uint32_t dst_ip = arp_msg.target_ip_address;
    EthernetAddress src_eth = arp_msg.sender_ethernet_address;
    arp_table_[src_ip] = { src_eth, 30000 };

    if ( arp_msg.opcode == ARPMessage::OPCODE_REQUEST && dst_ip == ip_address_.ipv4_numeric() ) {
      ARPMessage arp_reply { .opcode = ARPMessage::OPCODE_REPLY,
                             .sender_ethernet_address = ethernet_address_,
                             .sender_ip_address = ip_address_.ipv4_numeric(),
                             .target_ethernet_address = src_eth,
                             .target_ip_address = src_ip };

      EthernetHeader header { .dst = src_eth, .src = ethernet_address_, .type = EthernetHeader::TYPE_ARP };
      EthernetFrame send_frame { .header = header, .payload = serialize( arp_reply ) };
      transmit( send_frame );
    } else if ( arp_msg.opcode == ARPMessage::OPCODE_REPLY ) {
      arp_table_[src_ip] = { src_eth, 30000 };
      arp_5_.erase( src_ip );
      for ( auto it = arp_ip_waiting_.begin(); it != arp_ip_waiting_.end(); ++it ) {
        if ( it->first == src_ip )
          send_datagram( it->second, Address::from_ipv4_numeric( src_ip ) );
      }
      arp_ip_waiting_.erase( src_ip );
    }
  }
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void NetworkInterface::tick( const size_t ms_since_last_tick )
{
  for ( auto iter = arp_table_.begin(); iter != arp_table_.end(); ) {
    if ( iter->second.ttl <= ms_since_last_tick )
      iter = arp_table_.erase( iter );
    else {
      iter->second.ttl -= ms_since_last_tick;
      ++iter;
    }
  }

  for ( auto iter = arp_5_.begin(); iter != arp_5_.end(); ++iter ) {
    if ( iter->second <= ms_since_last_tick ) {
      ARPMessage arp_request { .opcode = ARPMessage::OPCODE_REQUEST,
                               .sender_ethernet_address = ethernet_address_,
                               .sender_ip_address = ip_address_.ipv4_numeric(),
                               .target_ethernet_address = {},
                               .target_ip_address = iter->first };

      EthernetHeader header {
        .dst = ETHERNET_BROADCAST, .src = ethernet_address_, .type = EthernetHeader::TYPE_ARP };
      EthernetFrame frame { .header = header, .payload = serialize( arp_request ) };
      transmit( frame );
      iter->second = 5000;
    } else
      iter->second -= ms_since_last_tick;
  }
}