#pragma once

//#include "geo_utils.h"

// #include "alloc/pool_stl.h"

namespace cg
{
    namespace details2
    {
        template<class points_t>
        struct curve_traits
        {
            typedef typename std::multimap<double, typename points_t::value_type> curve_t;
        };


        template<class points_t, class dist_f_t>
        typename curve_traits<points_t>::curve_t make_curve_map( points_t const& points, dist_f_t f )
        {
            typename curve_traits<points_t>::curve_t points_map;

            double len = 0;
            for (typename points_t::const_iterator it = points.begin(), end = boost::prior(points.end()); it != end; ++it)
            {
                points_map.insert(std::make_pair(len, *it));
                len += f(*it, *boost::next(it));
            }
            points_map.insert(std::make_pair(len, points.back()));

            return points_map;
        }
        
        template<class T = geo_point_2>
        struct distance2d_t
        {
            inline double operator()( T const& a, T const& b )
            {
                return cg::distance2d(a, b);
            }
        };

    }

    template<typename T>
    struct default_curve_lerp2
    {   
        typedef T  value_type;
        value_type operator()(const value_type &a, const value_type &b, double t) const
        {
            return cg::blend(a, b, t);
        }
    };


    template<typename T, typename L = default_curve_lerp2<cg::point_t<T,2>> >
    class curve2_t
    {
    public:
        typedef /*T*/cg::point_t<T,2>  value_type;
        typedef typename std::multimap<double, value_type/*, std::less<double>*/ > points_t;
        typedef L lerp_t;

    public:
        curve2_t()
        { }

        explicit curve2_t(const points_t &points)
            : points_(points)
        { }

        explicit curve2_t(points_t &&points)
            : points_(std::forward<points_t>(points))
        { }

        value_type operator()(double arg) const
        {
            return value(arg);
        }

        double length() const
        {
            return !points_.empty() ? boost::prior(points_.end())->first : 0;
        }

        value_type value(double arg) const
        {
            const auto its = bounds(arg);

            Assert(its.first != points_.end());

            const auto prev_it = its.first;
            const auto it = its.second;

            const auto a = prev_it->first;
            const auto b = it->first;
            Assert(cg::ge(b, a));

            const auto det = b - a;
            const auto t = cg::eq(det, 0.0) ? 0.5 : (arg - a) / det;

            return lerp(prev_it->second, it->second, t);
        }

        std::pair<typename points_t::const_iterator, typename points_t::const_iterator> bounds(double arg) const
        {
            const points_t &m = points_;
            if (m.size() <= 1)
                return std::make_pair(m.begin(), m.begin());

            auto it = m.lower_bound(arg);
            if (it == m.end())
                --it;

            if (it == m.begin())
                ++it;

            const auto prev_it = boost::prior(it);

            return std::make_pair(prev_it, it);
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

        curve2_t apply_offset(double offset) const
        {
            curve2_t res;
            BOOST_FOREACH(const auto &r, points_)
                res.points_.insert(res.points_.end(), make_pair(r.first + offset, r.second));
        }

        void append(const curve2_t &other) 
        {
            append(other, length());
        }

        void append(const curve2_t &other, double start_dist) 
        {
            Assert(cg::ge(start_dist, length()));
            BOOST_FOREACH(const auto &p, other.points())
                points().insert(points().end(), std::make_pair(start_dist + p.first, p.second));
        }

        void remove_equal_points(double tolerance = cg::epsilon<double>())
        {
            if (points().empty())
                return;

            auto src_it = points_.begin();
            for (auto it = std::next(src_it); it != points_.end();)
            {
                if (cg::eq(it->first, src_it->first, tolerance))
                {
                    it = points_.erase(it);
                }
                else
                {
                    src_it = it;
                    ++it;
                }
            }
        }

        std::vector<value_type> extract_values() const
        {
            std::vector<value_type> values;
            values.reserve(points_.size());
            BOOST_FOREACH(auto const &r, points_)
                values.push_back(r.second);
            return values;
        }

        //REFL_INNER(curve2_t)
        //  REFL_ENTRY(points_)
        //REFL_END()

    private:
        points_t points_;
    };


} // namespace cg