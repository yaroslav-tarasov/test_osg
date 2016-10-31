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

    template<typename T>
    inline T atanh (T x)
    {
        return (log(1+x) - log(1-x))/2;
    }

    template<typename T>
    inline T cbrt(T n)
    {
        if(n>0)
            return std::pow(n, 1/3.);
        else
            return -std::pow(n, 1/3.);
    }


    template<typename T, typename D>
    __forceinline D lerp_clamp( const T x, const T x0, const T x1, const D & y0, const D & y1 )
    {
        if (x <= x0)
            return y0;
        else if (x < x1)
            return lerp( x0, x1, y0, y1)(x);
        else
            return y1;
    }

    template<typename T> __forceinline T slerp01( const T x )
    {
        return (T(3) - T(2) * x) * x * x;
    };

    template<class T>
    __forceinline T clamp01( const T x )
    {
        return (x < 0) ? 0 : (x > 1) ? 1 : x;
    }
}