#pragma once

namespace extra_route
{
    template<class anchor_point_t>
    struct chart
    {
        typedef view<anchor_point_t> view_t;

        chart(view_t * v)
            : view_(v)
        {
        }

        void point_dragged(size_t idx, ani::point_pos const& new_pos)
        {
            if (view_->is_anchor(idx))
            {
                size_t anchor = view_->get_anchor(idx);

                anchor_point_t pnt = view_->anchor_points()[anchor];
                pnt.pos = ani::point_pos(new_pos.layer, new_pos);
                view_->set_anchor(anchor, pnt);
                if (anchor != 0)
                    view_->check_segment(anchor - 1);
                if (anchor != view_->anchor_points().size() - 1)
                    view_->check_segment(anchor);
            }
            else
            {
                size_t anchor = view_->get_anchor(idx);
                view_->add_anchor(anchor + 1, ani::point_pos(new_pos.layer, new_pos));
                view_->check_segment(anchor);
                if (anchor + 1 != view_->anchor_points().size() - 1)
                    view_->check_segment(anchor + 1);
            }
        }

        void point_added(size_t idx, ani::point_pos const& new_pos)
        {
            size_t anchor = view_->get_anchor(idx - 1);
            view_->add_anchor(anchor + 1, ani::point_pos(new_pos.layer, new_pos));
            view_->check_segment(anchor);
            if (anchor + 1 != view_->anchor_points().size() - 1)
                view_->check_segment(anchor + 1);
        }

        void point_removed(size_t idx)
        {
            if (view_->is_anchor(idx))
            {
                size_t anchor = view_->get_anchor(idx);
                view_->remove_anchor(anchor);
                if (anchor > 0 && anchor < view_->anchor_points().size())
                    view_->check_segment(anchor - 1);
            }
            else
            {
                // do nothing
            }
        }
    private:
        view_t * view_;
    };
}
