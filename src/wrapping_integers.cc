#include "wrapping_integers.hh"

using namespace std;

Wrap32 Wrap32::wrap( uint64_t n, Wrap32 zero_point )
{
  // Your code here.
  return Wrap32{uint32_t((n + zero_point.raw_value_) % (uint64_t(1) << 32))};
}

uint64_t Wrap32::unwrap( Wrap32 zero_point, uint64_t checkpoint ) const
{
  // Your code here.
  uint64_t wrap_modulus = uint64_t(1) << 32;
  uint64_t min_value = ((uint64_t)raw_value_ + wrap_modulus - zero_point.raw_value_) % wrap_modulus;
  uint64_t start = checkpoint/wrap_modulus;
  uint64_t candidate1 = min_value + start * wrap_modulus;
  uint64_t candidate2 = candidate1;
  if(candidate1 > checkpoint){
    if(start == 0)
      return candidate1;
    candidate2 = candidate1 - wrap_modulus;
    if(candidate1 - checkpoint > checkpoint - candidate2)
      return candidate2;
    else 
      return candidate1;  
  }
  else{
    candidate2 = candidate1 + wrap_modulus;
    if(checkpoint - candidate1 > candidate2 - checkpoint)
      return candidate2;
    else 
      return candidate1;
  }
}
