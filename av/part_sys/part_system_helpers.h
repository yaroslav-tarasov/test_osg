#pragma once


namespace av
{

namespace part_sys
{

    struct base_cpu_particle
    {
    public:
        base_cpu_particle() : age_(0), t_(0), lifeTime_(1), lifeTimeInv_(1) {}
        base_cpu_particle( cg::point_3f const & sp, float lt, float age ) : start_pos_(sp), cur_pos_(sp), age_(age), t_(0), lifeTime_(lt), lifeTimeInv_(1.f / lt) { t_ = age * lifeTimeInv_; }

        bool inc( float dt )
        {
            age_ += dt;
            t_ += lifeTimeInv_ * dt;
            return (age_ < lifeTime_);
        }

        const float & get_age()      const { return age_; }
        const float & lifetime()     const { return lifeTime_; }
        const float & lifetime_inv() const { return lifeTimeInv_; }
        const float & t()            const { return t_; }

        const cg::point_3f & start_pos() const { return start_pos_; }
        cg::point_3f & start_pos() { return start_pos_; }

        cg::point_3f & cur_pos() { return cur_pos_; }
        const cg::point_3f & cur_pos() const { return cur_pos_; }

    protected:

        cg::point_3f start_pos_;
        cg::point_3f cur_pos_;

        float age_;
        float t_;
        float lifeTime_;
        float lifeTimeInv_;
    };

} // namespace part_sys

} // namespace av
