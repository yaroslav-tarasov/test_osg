#pragma once 

namespace cg
{
    //
    template <class V>
    __forceinline typename V::value_type luminance_crt( V const & col )
    {
        return V::value_type(0.299f * col.x() + 0.587f * col.y() + 0.114f * col.z());
    }

    //
    template <typename V>
    __forceinline typename V::value_type luminance_lcd( V const & col )
    {
        return V::value_type(0.2127f * col.x() + 0.7152f * col.y() + 0.0722f * col.z());
    }

};