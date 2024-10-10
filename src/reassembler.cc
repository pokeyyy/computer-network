#include "reassembler.hh"

using namespace std;

void Reassembler::insert( uint64_t first_index, string data, bool is_last_substring )
{
  // Your code here.
  uint64_t available = output_.writer().available_capacity();
  uint64_t last_index = first_index + data.size() - 1;
  if ( is_last_substring ) {
    eof_flag = last_index + 1;
  }
  if ( first_index >= unassembled_index + available || last_index < unassembled_index )
    return;
  if ( first_index < unassembled_index ) {
    uint64_t erase_len = unassembled_index - first_index;
    first_index = unassembled_index;
    data.erase( 0, erase_len );
  }
  if ( last_index > unassembled_index + available - 1 ) {
    uint64_t erase_len = last_index - unassembled_index - available + 1;
    last_index = unassembled_index + available - 1;
    data.erase( data.end() - erase_len, data.end() );
  }

  bool need_insert = true;
  for ( auto i = buffer_.begin(); i != buffer_.end(); ++i ) {
    uint64_t head_now = i->first;
    uint64_t tail_now = i->first + i->second.size() - 1;
    if ( head_now <= first_index && tail_now >= last_index ) {
      need_insert = false;
      break;
    }
    if ( head_now < first_index && tail_now >= first_index ) {
      uint64_t erase_len = tail_now + 1 - first_index;
      first_index = tail_now + 1;
      data.erase( 0, erase_len );
    }
    if ( head_now >= first_index && tail_now <= last_index ) {
      string insert_str = data.substr( 0, head_now - first_index );
      pair<uint64_t, string> inserted( first_index, insert_str );
      buffer_.insert( i, inserted );
      uint64_t erase_len = tail_now + 1 - first_index;
      first_index = tail_now + 1;
      data.erase( 0, erase_len );
    }
    if ( head_now <= last_index && tail_now > last_index ) {
      last_index = head_now - 1;
      uint64_t substr_len = head_now - first_index;
      pair<uint64_t, string> inserted( first_index, data.substr( first_index, substr_len ) );
      buffer_.insert( i, inserted );
      need_insert = false;
      break;
    }
  }
  if ( need_insert ) {
    auto it = buffer_.begin();
    for ( ; it != buffer_.end(); ++it )
      if ( it->first > first_index ) {
        pair<uint64_t, string> element( first_index, data );
        buffer_.insert( it, element );
        break;
      }
    if ( it == buffer_.end() ) {
      pair<uint64_t, string> element( first_index, data );
      buffer_.push_back( element );
    }
  }

  auto it = buffer_.begin();
  while ( it->first == unassembled_index ) {
    output_.writer().push( it->second );
    unassembled_index += it->second.size();
    buffer_.pop_front();
    it = buffer_.begin();
  }
  if ( unassembled_index == eof_flag )
    output_.writer().close();
}

uint64_t Reassembler::bytes_pending() const
{
  // Your code here.
  uint64_t pend = 0;
  for ( auto i = buffer_.begin(); i != buffer_.end(); ++i )
    pend += i->second.size();
  return pend;
}
