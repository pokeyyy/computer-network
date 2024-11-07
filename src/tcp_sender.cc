#include "tcp_sender.hh"
#include "tcp_config.hh"
#include <iostream>
using namespace std;

uint64_t TCPSender::sequence_numbers_in_flight() const
{
  return outstanding_num_;
}

uint64_t TCPSender::consecutive_retransmissions() const
{
  return retransmission_num_;
}

void TCPSender::push( const TransmitFunction& transmit )
{
  uint64_t push_window;
  if(window_size_ == 0){
    push_window = 1;
    push_window = push_window <= outstanding_num_ ? 0 : push_window - outstanding_num_;
  }
  else 
    push_window = window_size_ <= outstanding_num_ ? 0 : window_size_ - outstanding_num_;

  while (push_window > 0){
    TCPSenderMessage send_msg;
    if (!syn_sent_){
      send_msg.SYN = true;
      syn_sent_ = true;
    }
    if (reader().has_error()){
      send_msg.RST = true;
    }
    if ( fin_sent_ )
      return;
    send_msg.seqno = Wrap32::wrap(seqno_,isn_);
    size_t payload_size = min(TCPConfig::MAX_PAYLOAD_SIZE, push_window);

    while(send_msg.sequence_length() < payload_size && reader().bytes_buffered()){
      string_view str = reader().peek();
      uint64_t bytes_read = min(str.size(),payload_size - send_msg.sequence_length());
      send_msg.payload += str.substr(0, bytes_read);
      input_.reader().pop(bytes_read);
      payload_size -= bytes_read;
    }
    if (reader().is_finished() && send_msg.sequence_length() < push_window){
      fin_sent_ = send_msg.FIN = true;
    }

    if ( send_msg.sequence_length() == 0 )
      return;

    seqno_ += send_msg.sequence_length();
    outstanding_num_ += send_msg.sequence_length();
    outstanding_segments_.push(send_msg);
    transmit(send_msg);

    if (!timer_flag_) {
      timer_flag_ = true;
      expire_time_ = RTO_;
      timer_ = 0;
    }
    if(window_size_ == 0){
    push_window = 1;
    push_window = push_window <= outstanding_num_ ? 0 : push_window - outstanding_num_;
  }
  else 
    push_window = window_size_ <= outstanding_num_ ? 0 : window_size_ - outstanding_num_;
  }

}

TCPSenderMessage TCPSender::make_empty_message() const
{
  string str = "";
  return { Wrap32::wrap( seqno_, isn_ ), false, str, false, reader().has_error() };
}

void TCPSender::receive( const TCPReceiverMessage& msg )
{
  window_size_ = msg.window_size;
  if(msg.RST){
    writer().set_error();
    return;
  }
  if(!msg.ackno.has_value())
    return;
  uint64_t abs_ackno = msg.ackno.value().unwrap(isn_,ackno_);
  if(abs_ackno > seqno_ || abs_ackno <= ackno_)
    return;

  ackno_ = abs_ackno;
  RTO_ = initial_RTO_ms_;
  retransmission_num_ = 0;

  while(!outstanding_segments_.empty()){
    auto flight_msg = outstanding_segments_.front();
    if(flight_msg.seqno.unwrap(isn_,ackno_) + flight_msg.sequence_length() > abs_ackno)
      break;
    outstanding_num_ -= flight_msg.sequence_length();
    outstanding_segments_.pop();
  } 
  if(!outstanding_segments_.empty()){
    expire_time_ = RTO_;
    timer_ = 0;
  }
  else{
    timer_flag_ = false;
  }
}

void TCPSender::tick( uint64_t ms_since_last_tick, const TransmitFunction& transmit )
{
  timer_ += ms_since_last_tick;
  if ( timer_flag_ && timer_ >= expire_time_ ) {
    transmit( outstanding_segments_.front() );
    if ( window_size_ != 0 ) {
      RTO_ *= 2;
    }
    retransmission_num_++;
    timer_ = 0;
    expire_time_ = RTO_;
  }
}