#pragma once

namespace extra_route
{
    template<class anchor_point_t>
    struct chart;

    template<class anchor_point_t>
    inline ani::point_pos anchor_point_get_pos(anchor_point_t const& pnt);

    template<class anchor_point_t>
    inline void anchor_point_set_pos(anchor_point_t & pnt, ani::point_pos const& pos);

    template<>
    inline ani::point_pos anchor_point_get_pos<ani::point_pos>(ani::point_pos const& pnt)
    {
        return pnt;
    }

    template<>
    inline void anchor_point_set_pos<ani::point_pos>(ani::point_pos & pnt, ani::point_pos const& pos)
    {
        pnt = pos ;
    }


    template<class anchor_point_t>
    struct view
    {
        friend struct chart<anchor_point_t>;
        
        typedef 
            std::vector<anchor_point_t>   anchor_points_t;

        view()
        {
        }

        view(anchor_points_t const&  anchor_points)
            : anchor_points_(anchor_points)
        {
            free_points_.resize(anchor_points_.size());
            recalc_anchor_ref();
        }

    protected:
        virtual std::vector<ani::point_pos> do_check_segment(anchor_point_t const& /*p*/, anchor_point_t const& /*q*/)
        {
            return std::vector<ani::point_pos>();
        }

        std::vector<anchor_point_t> const&   anchor_points() const
        {
            return anchor_points_;
        }


        void check_segments()
        {
            if(anchor_points_.size()>0)
            for (size_t i = 0; i < anchor_points_.size() - 1; ++i)
                check_segment(i);
        }


        std::vector<ani::point_pos> all_points() const
        {
            std::vector<ani::point_pos> points;
            for (size_t i = 0; i < anchor_points_.size(); ++i)
            {
                points.push_back(anchor_points_[i].pos);
                for (size_t j = 0; j < free_points_[i].size(); ++j)
                    points.push_back(free_points_[i][j]);
            }

            return std::vector<ani::point_pos>(std::move(points));
        }

        bool is_anchor(size_t idx) const
        {
            return anchor_ref_[idx].second;
        }

        size_t get_anchor(size_t idx) const
        {
            return idx < anchor_ref_.size() ? anchor_ref_[idx].first : anchor_points_.size() - 1;
        }

        void set_anchor(size_t where, anchor_point_t const& pnt)
        {                 
            anchor_points_[where] = pnt;
        }

    private:
        void check_segment(size_t idx)
        {
            Assert(idx < anchor_points_.size() - 1);

            std::vector<ani::point_pos> free_points = std::move(do_check_segment(anchor_points_[idx], anchor_points_[idx+1]));
            set_free_points(idx, free_points);
        }

        size_t anchor_idx(size_t anchor) const
        {
            Assert(anchor < anchor_points_.size());
            size_t idx = 0;
            for (size_t i = 0; i != anchor; ++i)
            {
                ++idx;
                for (size_t j = 0; j < free_points_[i].size(); ++j)
                    ++idx;
            }

            return idx;
        }

        void set_free_points(size_t anchor_seg, std::vector<ani::point_pos> const& free_points)
        {
            Assert(anchor_seg < anchor_points_.size() - 1);

            free_points_[anchor_seg] = free_points;
            recalc_anchor_ref();
        }



        void add_anchor(size_t where, ani::point_pos const& pos)
        {
            anchor_points_.insert(anchor_points_.begin() + where, anchor_point_t(pos));
            free_points_.insert(free_points_.begin() + where, std::vector<ani::point_pos>());
            recalc_anchor_ref();
        }

        void remove_anchor(size_t where)
        {
            anchor_points_.erase(anchor_points_.begin() + where);
            free_points_.erase(free_points_.begin() + where);
            recalc_anchor_ref();
        }

        void recalc_anchor_ref()
        {
            anchor_ref_.resize(0);
            for (size_t i = 0; i < anchor_points_.size(); ++i)
            {
                anchor_ref_.push_back(std::make_pair(i, true));
                for (size_t j = 0; j < free_points_[i].size(); ++j)
                    anchor_ref_.push_back(std::make_pair(i, false));
            }
        }

    private:
        std::vector<anchor_point_t>                        anchor_points_;
        std::vector<std::vector<ani::point_pos>>           free_points_;
        std::vector<std::pair<binary::size_type, bool>>    anchor_ref_;

    private:
        REFL_INNER(view)
            REFL_ENTRY(anchor_points_)
            REFL_ENTRY(free_points_)
            REFL_ENTRY(anchor_ref_)
        REFL_END()
    };
}