#include "wrapping_integers.hh"
#include <cstdint>
#include <cstdlib>
#include <math.h>

using namespace std;

Wrap32 Wrap32::wrap( uint64_t n, Wrap32 zero_point )
{
    return  zero_point + static_cast<uint32_t>(n) ;
}

uint64_t Wrap32::unwrap( Wrap32 zero_point, uint64_t checkpoint ) const
{
    uint32_t sub = this->raw_value_ - wrap(checkpoint , zero_point).raw_value_;
    uint64_t ans = checkpoint + static_cast<uint64_t>(sub);
    if (sub >= (1ul << 31) && ans >= (1ul << 32)){
        ans -= (1ul << 32);
    }
    return ans;
}
