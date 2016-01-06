#pragma once

#include "common/randgen.h"


namespace av
{

namespace part_sys
{

    // CPU processed particles queue
    template<typename TParticle, unsigned blocksize = 1024U>
    struct cpu_part_queue : public base_particle_queue<TParticle, blocksize>
    {
        typedef base_particle_queue<TParticle, blocksize> base_queue_t;

        // update queue
        template<typename Func>
        __forceinline bool update( float dt, Func const & update_proc )
        {
            bool changed = false;
            for (auto part = base_queue_t::begin(), it_end = base_queue_t::end(); part != it_end; )
            {
                if (part->inc(dt))
                {
                    update_proc(*part, dt);
                    ++part;
                }
                else
                {
                    part = base_queue_t::erase(part);
                    changed = true;
                }
            }
            return changed;
        }
    };

    // helper which traces time and position, and emits particles smoothly and correctly
    template <typename cpuqueue>
    struct sfx_pos_time_emitter
    {
        sfx_pos_time_emitter() : dt(0) {}

        __forceinline cpuqueue & get_queue() { return queue_; }
        __forceinline const cpuqueue & get_queue() const { return queue_; }

        // make all tracing stuff and update current queue
        template <typename upd_proc_t>
        void trace_and_update( float sim_time, cg::point_3f const & cur_trans, float break_dist, upd_proc_t const & update_proc )
        {
            // get time delta
            dt = visit_last_ ? sim_time - *visit_last_ : 0.f;
            set_visit_last(sim_time);
            if (cg::eq_zero(dt))
                return;

            // check time rewind
            if (dt < 0.f)
            {
				force_log fl;       
				LOG_ODS_MSG( "trace_and_update sim_time= " << sim_time << " *visit_last_= " <<*visit_last_ <<"\n");
                reset_visit_last();
                reset_emanation_last();
                reset_emit_pos_last();
                queue_.clear();
                return;
            }

            // update position step
            cur_trans_ = cur_trans;
            if (!pos_emit_prev_)
                set_emit_pos_last(cur_trans_);
            prev_trans_ = *pos_emit_prev_;
            if (cg::distance_sqr(cur_trans_, prev_trans_) > cg::sqr(break_dist))
            {
                reset_emanation_last();
                set_emit_pos_last(cur_trans_);
                prev_trans_ = cur_trans_;
                queue_.clear();
            }

            // queue update
            queue_.update(dt, update_proc);
        }

        // emit new particles with respect to smooth positions and time flow
        template <typename emit_proc_t, typename upd_proc_t>
        void emit_new_particles( float time_int, float dist_int, float break_time, emit_proc_t const & emit_proc, upd_proc_t const & update_proc )
        {
            if (!visit_last_) // time rewind
                return;

            // current time from last emanation
            if (!emanation_last_ || (*visit_last_ - *emanation_last_ > break_time))
                set_emanation_last(*visit_last_);
            float time_from_emanation = *visit_last_ - *emanation_last_;

            // smooth time and position interpolation
            float overall_to_emit = time_from_emanation * time_int + cg::distance(cur_trans_, prev_trans_) * dist_int;
            const float emit_unit_step = (overall_to_emit >= 1.f) ? 1.f / floor(overall_to_emit) : 0.f;
            const float emit_time_step = emit_unit_step * time_from_emanation;

            // need to emit new particles?
            float pos_lerp_k = 0;
            while (overall_to_emit >= 1.f)
            {
                // shift time
                set_emanation_last(*emanation_last_ + emit_time_step);
                time_from_emanation -= emit_time_step;
                // get pos
                pos_lerp_k += emit_unit_step;
                auto const world_pos = cg::lerp01(prev_trans_, cur_trans_, pos_lerp_k);
                set_emit_pos_last(world_pos);
                // emission and all relevant updates
                auto new_particle = emit_proc(world_pos, *emanation_last_, time_from_emanation, rndgen_);
                update_proc(new_particle, time_from_emanation);
                queue_.push_front(new_particle);
                // goto next p
                overall_to_emit -= 1.0f;
            }
        }

        // simultaneous particles injection
        template <typename emit_proc_t, typename upd_proc_t>
        void inject_particles( float timestamp, unsigned cnt, emit_proc_t const & emit_proc, upd_proc_t const & update_proc )
        {
            set_emanation_last(timestamp);
            set_emit_pos_last(cur_trans_);
            const float tme = *visit_last_ - timestamp;
            for (unsigned i = 0; i < cnt; ++i)
            {
                auto new_particle = emit_proc(cur_trans_, timestamp, tme, rndgen_);
                update_proc(new_particle, tme);
                queue_.push_front(new_particle);
            }
        }

    public:
        void reset_emit_pos_last() { pos_emit_prev_ = boost::none; }
        void set_emit_pos_last( cg::point_3f const & p ) { pos_emit_prev_ = p; }

        void reset_visit_last() { visit_last_ = boost::none; }
        void set_visit_last( float time ) { visit_last_ = time; }

        void reset_emanation_last() { emanation_last_ = boost::none; }
        void set_emanation_last( float time ) { emanation_last_ = time; }

    private:
        cpuqueue queue_;

        simplerandgen rndgen_;

        boost::optional<cg::point_3f> pos_emit_prev_;
        boost::optional<float> visit_last_;
        boost::optional<float> emanation_last_;

        float dt;
        cg::point_3f prev_trans_;
        cg::point_3f cur_trans_;
    };

} // namespace part_sys

} // namespace av
