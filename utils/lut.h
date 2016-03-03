#pragma once 

namespace Utils
{

    template<typename key_type, typename value_type>
    struct LookupTable
    {
        const key_type min, max;
        const unsigned count;
        value_type * const data;
        const key_type scale, bias;

        inline LookupTable( key_type min, key_type max, unsigned count, value_type (*func)( key_type key ) )
            : min(min)
            , max(max)
            , count(count)
            , data(new value_type[count])
            , scale(key_type(count - 1) / (max - min))
            , bias(-min * scale)
        {
            for (unsigned i = 0; i < count; i++)
                data[i] = func(key_type(i) / (count - 1) * (max - min) + min);
        }

        inline ~LookupTable()
        {
            delete[] data;
        }

        inline value_type operator()( key_type key ) const
        {
            const key_type key_idx = key * scale + bias;
            const int idx = int(key_idx);
            if (idx <= 0)
                return data[0];
            else if (unsigned(idx) >= count - 1)
                return data[count - 1];
            else
                return cg::lerp01( data[idx], data[idx + 1],key_idx - idx);
        }
    };

} // namespace utils