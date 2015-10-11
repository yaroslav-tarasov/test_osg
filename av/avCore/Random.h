#pragma once

#undef min
#undef max

#include <limits>

namespace avCore
{

// ported from boost
template<typename UIntType, int w, int n, int m, int r, UIntType a, int u, int s, UIntType b, int t, UIntType c, int l, UIntType val>
struct mersenne_twister_t
{
    typedef UIntType result_type;
    static const int word_size = w;
    static const int state_size = n;
    static const int shift_size = m;
    static const int mask_bits = r;
    static const UIntType parameter_a = a;
    static const int output_u = u;
    static const int output_s = s;
    static const UIntType output_b = b;
    static const int output_t = t;
    static const UIntType output_c = c;
    static const int output_l = l;

    static const bool has_fixed_range = false;

    inline mersenne_twister_t()
    {
        seed(UIntType(5489));
    }

    inline explicit mersenne_twister_t( UIntType value )
    {
        seed(value);
    }
    
    inline void seed( UIntType value )
    {
        const UIntType mask = ~0u;
        x[0] = value & mask;
        for (i = 1; i < n; i++) {
            // See Knuth "The Art of Computer Programming" Vol. 2, 3rd ed., page 106
            x[i] = (1812433253UL * (x[i-1] ^ (x[i-1] >> (w-2))) + i) & mask;
        }
    }

    inline result_type operator()()
    {
        if(i == n)
            twist(0);
        else if(i >= 2*n)
            twist(1);
        // Step 4
        UIntType z = x[i];
        ++i;
        z ^= (z >> u);
        z ^= ((z << s) & b);
        z ^= ((z << t) & c);
        z ^= (z >> l);
        return z;
    }

private:
    void twist( int block )
    {
        const UIntType upper_mask = (~0u) << r;
        const UIntType lower_mask = ~upper_mask;

        if(block == 0) {
            for(int j = n; j < 2*n; j++) {
                UIntType y = (x[j-n] & upper_mask) | (x[j-(n-1)] & lower_mask);
                x[j] = x[j-(n-m)] ^ (y >> 1) ^ (y&1 ? a : 0);
            }
        } else if (block == 1) {
            // split loop to avoid costly modulo operations
            {  // extra scope for MSVC brokenness w.r.t. for scope
                for(int j = 0; j < n-m; j++) {
                    UIntType y = (x[j+n] & upper_mask) | (x[j+n+1] & lower_mask);
                    x[j] = x[j+n+m] ^ (y >> 1) ^ (y&1 ? a : 0);
                }
            }

            for(int j = n-m; j < n-1; j++) {
                UIntType y = (x[j+n] & upper_mask) | (x[j+n+1] & lower_mask);
                x[j] = x[j-(n-m)] ^ (y >> 1) ^ (y&1 ? a : 0);
            }
            // last iteration
            UIntType y = (x[2*n-1] & upper_mask) | (x[0] & lower_mask);
            x[n-1] = x[m-1] ^ (y >> 1) ^ (y&1 ? a : 0);
            i = 0;
        }
    }

    // state representation: next output is o(x(i))
    //   x[0]  ... x[k] x[k+1] ... x[n-1]     x[n]     ... x[2*n-1]   represents
    //  x(i-k) ... x(i) x(i+1) ... x(i-k+n-1) x(i-k-n) ... x[i(i-k-1)]
    // The goal is to always have x(i-n) ... x(i-1) available for
    // operator== and save/restore.

    UIntType x[2 * n]; 
    int i;
};

typedef mersenne_twister_t<unsigned int, 32, 351, 175, 19, 0xccab8ee7, 11, 7, 0x31b6ab00, 
    15, 0xffe50000, 17, 0xa37d3c92> mersenne_twister_11213b;

// validation by experiment from mt19937.c
typedef mersenne_twister_t<unsigned int, 32, 624, 397, 31, 0x9908b0df, 11, 7, 0x9d2c5680, 
    15, 0xefc60000, 18, 3346425566U> mersenne_twister_19937;

template <typename scalar_type, typename generator_type>
inline scalar_type random_range( generator_type & gen, scalar_type x0, scalar_type x1 )
{
    static const generator_type::result_type 
        genMin = std::numeric_limits<generator_type::result_type>::min(),
        genMax = std::numeric_limits<generator_type::result_type>::max();

    // it's not correct for integral types
    return x0 + (x1 - x0) * (gen() - genMin) / (genMax - genMin);
}

template <typename scalar_type, typename generator_type>
inline scalar_type random_deviation( generator_type & gen, scalar_type average, scalar_type deviation )
{
    return random_range(gen, average - deviation, average + deviation);
}

template <typename generator_type>
inline int random_sign( generator_type & gen )
{
    return ((gen() & 1) << 1) - 1;
}


//
// Fast random generator
//

class RandomNumber
{

public:

    // constructors
    RandomNumber() : m_dwSeed(0) {}
    RandomNumber(size_t seed) : m_dwSeed(seed) {}

    // reseed function
    void reseed(size_t seed) const { m_dwSeed = seed; }

    // floating point randoms
    __forceinline float random_unit() const
    {
        static const float g_Mul = 1.0f / 65535.f;
        return g_Mul * random_16bit();
    }
    __forceinline float random_unit_signed() const
    {
        static const float g_Mul = 2.0f / 65535.f;
        return g_Mul * random_16bit() - 1.0f;
    }
    __forceinline float random_dev(float val, float dev) const
    {
        return val + random_unit_signed() * dev;
    }
    __forceinline float random_range(float min_val, float max_val) const
    {
        return min_val + random_unit() * (max_val - min_val);
    }

    // integer randoms
    __forceinline size_t random_8bit() const 
    {
        seed_iterate();
        return (m_dwSeed >> 16) & 0xFF;
    }
    __forceinline size_t random_16bit() const
    {
        seed_iterate();
        return (m_dwSeed >> 8) & 0xFFFF;
    }
    __forceinline size_t random_32bit() const
    {
        seed_iterate();
        return m_dwSeed;
    }

    __forceinline void seed_iterate() const
    {
        m_dwSeed = (214013 * m_dwSeed + 2531011);
    }

private:

    // current seed
    mutable size_t m_dwSeed;
};

}

