#include "byte_stream.hh"

using namespace std;

ByteStream::ByteStream( uint64_t capacity ) : capacity_( capacity ) {}

bool Writer::is_closed() const
{
  // Your code here.
  return closed_;
}

void Writer::push( string data )
{
  // Your code here.
  uint64_t available = min(available_capacity(), data.size() );
  for(int i = 0;available > 0;i ++){
    buffer_.push_back(data[i]);
    available--;  
    written_byte++;
  }
}

void Writer::close()
{
  // Your code here.

  closed_ = true;
}

uint64_t Writer::available_capacity() const
{
  // Your code here.
  return capacity_ - (written_byte - read_byte);
}

uint64_t Writer::bytes_pushed() const
{
  // Your code here.
  return written_byte;
}

bool Reader::is_finished() const
{
  // Your code here.
  return closed_ && buffer_.empty();
}

uint64_t Reader::bytes_popped() const
{
  // Your code here.
  return read_byte;
}

string_view Reader::peek() const
{
  // Your code here.
  return buffer_;
}

void Reader::pop( uint64_t len )
{
  // Your code here.
  if(len > buffer_.size()){
    set_error();
    return;
  }
  buffer_.erase(buffer_.begin(),buffer_.begin() + len);
  read_byte += len;
}

uint64_t Reader::bytes_buffered() const
{
  // Your code here.
  return written_byte - read_byte;
}
