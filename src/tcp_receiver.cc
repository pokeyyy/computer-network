#include "tcp_receiver.hh"
#include <type_traits>
#include <iostream>

using namespace std;

void TCPReceiver::receive( TCPSenderMessage message )
{
  // Your code here.
  if(message.RST){
    reader().set_error();
    return;
  }
  if(!already_syn && !message.SYN)
    return;
  if (message.SYN && !already_syn) {
    isn_ = message.seqno;
    already_syn = true;
  }
  uint64_t absolute_seqno = message.seqno.unwrap(isn_, writer().bytes_pushed() + 1);
  if (!message.payload.empty()) {
    reassembler_.insert(absolute_seqno + + static_cast<uint64_t>( message.SYN ) - 1, message.payload, message.FIN);
  }
}

TCPReceiverMessage TCPReceiver::send() const
{
  // Your code here.
  uint16_t available = writer().available_capacity() > UINT16_MAX? static_cast<uint16_t>( UINT16_MAX ): static_cast<uint16_t>( writer().available_capacity() );
  uint64_t ackno = writer().bytes_pushed()+1;
  if(already_syn)
    return TCPReceiverMessage{Wrap32::wrap(ackno,isn_),available,writer().has_error()};
  else 
    return TCPReceiverMessage{nullopt,available,writer().has_error()};
}
