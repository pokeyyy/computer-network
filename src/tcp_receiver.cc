#include "tcp_receiver.hh"
#include <iostream>
#include <type_traits>

using namespace std;

void TCPReceiver::receive( TCPSenderMessage message )
{
  // Your code here.
  if ( message.RST ) {
    reader().set_error();
    return;
  }
  if ( !already_syn && !message.SYN )
    return;
  if ( message.SYN && !already_syn ) {
    isn_ = message.seqno;
    already_syn = true;
  }
  uint64_t absolute_seqno = message.seqno.unwrap( isn_, writer().bytes_pushed() );
  reassembler_.insert( absolute_seqno + uint64_t( message.SYN ) - 1, message.payload, message.FIN );
}

TCPReceiverMessage TCPReceiver::send() const
{
  // Your code here.
  uint16_t available
    = writer().available_capacity() > UINT16_MAX ? UINT16_MAX : uint16_t( writer().available_capacity() );
  uint64_t ackno = writer().bytes_pushed() + 1 + uint64_t( writer().is_closed() );
  if ( already_syn )
    return TCPReceiverMessage { Wrap32::wrap( ackno, isn_ ), available, writer().has_error() };
  else
    return TCPReceiverMessage { nullopt, available, writer().has_error() };
}