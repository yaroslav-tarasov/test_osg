#pragma once

//#include "geo_utils.h"

#include "alloc/pool_stl.h"

namespace cg
{

    template<typename T, typename L = default_curve_lerp<T> >
    class curve2_t
    {
    public:
        typedef T value_type;
        // typedef typename ph_map<double, value_type, std::less<double> >::map_t points_t;

        typedef typename std::vector< double > lengths_t;
        typedef typename std::vector< value_type> points_t;
        typedef L lerp_t;

    private:

        struct cache
        {
            cache(double lower=0, double hi=0)
                : _low(lower)
                , _hi(hi)
                , _dist(0)
            {}

            inline void operator()(double lower, double hi)
            {
                _low = lower;
                _hi = hi;
            }

            inline std::pair<typename points_t::const_iterator, typename points_t::const_iterator> operator()(
                std::pair<typename lengths_t::const_iterator, typename lengths_t::const_iterator>&  len,
                std::pair<typename points_t::const_iterator, typename points_t::const_iterator>&    val,
                double                                                                             dist
                )
            {
                _low       = *len.first;
                _hi        = *len.second;
                _lower_val = *val.first;
                _hi_val    = *val.second;
                _dist      = dist;

                return val;
            }

            inline bool eq(double lower, double hi)
            {
                return _low == lower && _hi == hi;
            }

            inline bool in_interval(double value)
            {
                return _low < value && _hi >= value;
            }

            double     _low;
            double     _hi; 
            value_type _lower_val;
            value_type _hi_val;
            double     _dist;
        };

    public:
        curve2_t()
        { }

        //explicit curve2_t(const points_t &points)
        //    : points_(points)
        //{ }

        //explicit curve2_t(points_t &&points)
        //    : points_(std::forward<points_t>(points))
        //{ }

        inline value_type operator()(double arg) const
        {
            return value(arg);
        }

        inline double length() const
        {
            return !length_.empty() ? *boost::prior(length_.end()) : 0;
        }

        value_type value(double arg) const
        {
            double a = 0;
            double b = 0;
            points_t::const_iterator prev_it;
            points_t::const_iterator it;
            bool  in_int = cache_.in_interval(arg);
            if (!in_int)
            {
                const points_t &m  = points_;
                const lengths_t &l = length_;
                
                
                const auto its = bounds(arg);

                Assert(its.first != points_.end());

                /*const auto*/ prev_it = its.first;
                /*const auto*/ it = its.second;
                
                

                /*const auto*/ a = *(l.begin() + (prev_it - m.begin()));
                /*const auto*/ b = *(l.begin() + (it - m.begin()));
            }
            else
            {
                a = cache_._low;
                b = cache_._hi;
            }

            Assert(cg::ge(b, a));

            const auto det = b - a;
            const auto t = cg::eq(det, 0.0) ? 0.5 : (arg - a) / det;

            return lerp(in_int?cache_._lower_val:*prev_it, in_int?cache_._hi_val:*it, t);
        }



        std::pair<typename points_t::const_iterator, typename points_t::const_iterator> bounds(double arg) const
        {
            const points_t &m  = points_;
            const lengths_t &l = length_;
            if (m.size() <= 1)
                return cache_(std::make_pair(l.begin(), l.begin()),std::make_pair(m.begin(), m.begin()),0);

            auto it = std::lower_bound(l.begin() + cache_._dist,l.end(),arg);
            if (it == l.end())
                --it;

            if (it == l.begin())
                ++it;

            const auto prev_it = boost::prior(it);
            const auto dist = (prev_it - l.begin());
            return cache_(std::make_pair(prev_it, it),std::make_pair(m.begin() + dist,m.begin() + dist + 1 ),dist);
        }

        void insert(const std::pair<double, value_type>& p)
        {
            points_.push_back(p.second);
            length_.push_back(p.first);
        }

        const points_t &points() const
        {
            return points_;
        }

        points_t &points()
        {
            return points_;
        }

        value_type lerp(const value_type &a, const value_type &b, double t) const
        {
            lerp_t l;
            return l(a, b, t);
        }

        //curve_t apply_offset(double offset) const
        //{
        //    curve_t res;
        //    BOOST_FOREACH(const auto &r, points_)
        //        res.points_.insert(res.points_.end(), std::make_pair(r.first + offset, r.second));
        //}

        void append(const curve2_t &other) 
        {
            append(other, length());
        }

        void append(const curve2_t &other, double start_dist) 
        {
            Assert(cg::ge(start_dist, length()));
            //BOOST_FOREACH(const auto &p, other.points())
            //    points().insert(points().end(), std::make_pair(start_dist + p.first, p.second));
            points_.insert(points_.end(), other.points_.begin(), other.points_.end()); 
            BOOST_FOREACH(const auto &p, other.length_)
                length_.push_back(start_dist + p);
            
            //length_.insert(length_.end(), other.length_.begin(), other.length_.end());
        }

        //void remove_equal_points(double tolerance = cg::epsilon<double>())
        //{
        //    if (points().empty())
        //        return;

        //    auto src_it = points_.begin();
        //    for (auto it = std::next(src_it); it != points_.end();)
        //    {
        //        if (cg::eq(it->first, src_it->first, tolerance))
        //        {
        //            it = points_.erase(it);
        //        }
        //        else
        //        {
        //            src_it = it;
        //            ++it;
        //        }
        //    }
        //}

        std::vector<value_type> extract_values() const
        {
            return points_;
        }


    private:
        points_t         points_;
        lengths_t        length_;
        mutable  cache    cache_;
    };

















    template<typename T, typename LERP = default_curve_lerp<T>, typename DIST = details::distance2d_t>
    class point_curve_2_ext
        : public curve_t<T, LERP>
    {
    };

} // namespace cg