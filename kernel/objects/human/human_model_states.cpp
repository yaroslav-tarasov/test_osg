#include "stdafx.h"
#include "precompiled_objects.h"

#include "human_model_states.h"
#include "human_model.h"

#include "geometry/filter.h"

template<typename T,int N>
struct ff
{

    double next(T value )
    {
        T sum=value;
        int i;
        for (i=0;i<N;i++)
        {
           sum+=val_[i];
           if(i<(N-1)) 
               val_[i] = val_[i+1];
        }
        val_[i] = sum/N;
        return val_[i];
    }

    std::array<T,N> val_;
};

namespace human
{

follow_route_state::follow_route_state(simple_route::info_ptr route)
    : route_(route)
    , dcourse_(0)
{
}

void follow_route_state::update(model * self, double dt)
{
    if (!route_)
        return;

    FIXME("И ведь такого по всему коду(raw const)")
    double const model_calc_step = 0.1;

    cg::geo_base_2  cur_pos     = self->pos();
    double          cur_course  = self->course();
    double          cur_speed   = self->speed();

    LOG_ODS_MSG( "follow_route_state::update:  cur_pos:  x:  "  << cur_pos.lat << "    y: " << cur_pos.lon << "\n" );

    size_t steps = cg::floor(dt / model_calc_step + model_calc_step * .1);

    double max_speed = 0;
    for (size_t i = 0; i < steps; ++i)
    {
        cur_pos += cg::point_2(cg::polar_point_2(1., cur_course)) * cur_speed * model_calc_step;
        cur_course += dcourse_ * model_calc_step;

        double const prediction = 10 * model_calc_step;

        double len = route_->closest(cg::geo_point_3(cur_pos, 0));

        double nominal_speed = route_->interpolate_speed(len);
        double desired_len = cg::bound(len + nominal_speed * prediction, 0., route_->length());
         

        cg::geo_point_2 desired = route_->interpolate(desired_len);
        
        LOG_ODS_MSG( "follow_route_state::update:  desired:  x:  "  << desired.lat << "    y: " << desired.lon << "\n"
                     << "len:  "  << len << "  desired_len:  "  << desired_len << "\n"
                     << "nominal_speed:  "  << nominal_speed << "  desired_len:  "  << desired_len << "\n"
            );
        
        cg::point_2 dir = cur_pos(desired);

        double desired_course = cg::polar_point_2(dir).course;

        double max_dcourse = cg::clamp(0., nominal_speed, 0., 100.)(cur_speed);

        dcourse_  = filter::BreakApproachSpeed<cg::degree180_value>(cur_course, desired_course, dcourse_, max_dcourse, 100., model_calc_step, 1.1);
        cur_speed = filter::BreakApproachSpeed(0., (route_->length() - len), cur_speed, nominal_speed, 10., model_calc_step, 1.1);

        max_speed = nominal_speed;
    }

    self->set_state(state_t(cur_pos, cur_course, cur_speed));
    self->set_max_speed(max_speed);
}

follow_curve_state::follow_curve_state(cg::geo_curve_2 const& route, double end_course, bool with_airtow)
    : route_(route)
    , dcourse_(0)
    , with_airtow_(with_airtow)
    , end_course_(end_course)
    , end_(false)
{
}

void follow_curve_state::update(model * self, double dt)
{
    double const model_calc_step = cfg().model_params.msys_step;

    cg::geo_base_2 cur_pos = self->pos();
    double cur_course = self->course();
    double cur_speed = self->speed();
    
    size_t steps = cg::floor(dt / model_calc_step + model_calc_step * .1);

    double max_speed = 0;
    bool end = false;
    for (size_t i = 0; i < steps; ++i)
    {
        cur_pos += cg::point_2(cg::polar_point_2(1., cur_course)) * cur_speed * model_calc_step;
        cur_course += dcourse_ * model_calc_step;

        double const prediction = 10 * model_calc_step;

        double len = route_.closest(cur_pos);

        const double nominal_speed = with_airtow_ ? 3. : 10;
        double desired_len = cg::bound(len + nominal_speed * prediction, 0., route_.length());

        cg::geo_point_2 desired = route_(desired_len);

        cg::point_2 dir = cur_pos(desired);

        double desired_course = cg::polar_point_2(dir).course;

        double max_dcourse = cg::clamp(0., nominal_speed, 0., 100.)(cur_speed);

        dcourse_ = filter::BreakApproachSpeed<cg::degree180_value>(cur_course, desired_course, dcourse_, max_dcourse, 100., model_calc_step, 1.1);
        cur_speed = filter::BreakApproachSpeed(0., (route_.length() - len), cur_speed, nominal_speed, 10., model_calc_step, 1.1);

        if (cg::eq(len, route_.length(), 0.01))
        {
            cur_course = end_course_;
            end = true;
        }

        max_speed = nominal_speed;
    }

    if (end)
    {
        if (cg::distance2d(self->phys_pos(), cur_pos) < 0.5)
        {
            self->set_course_hard(cur_course);
            end_ = true;
        }
    }


    self->set_state(state_t(cur_pos, cur_course, cur_speed));
    self->set_max_speed(max_speed);
}


go_to_pos_state::go_to_pos_state(cg::geo_point_2 const& pos, optional<double> course, bool with_airtow)
    : target_pos_(pos)
    , target_course_(course)
    , dcourse_(0)
    , with_airtow_(with_airtow)
{
}

void go_to_pos_state::update(model * self, double dt)
{
    const double nominal_speed = with_airtow_ ? 3. : 10;

    cg::geo_base_2 cur_pos    = self->pos();
    double         cur_course = self->course();
    double         cur_speed  = self->speed();
    
    std::stringstream cstr;

    cstr << std::setprecision(8) 
        << "x:  "         << cur_pos.lat
        << "    y: "      << cur_pos.lon
        << "    curs :  " << cur_course 
        << "    cur_speed:  " << cur_speed 
        << "\n" ;

    OutputDebugString(cstr.str().c_str());

    double max_speed = 0;

    cur_pos += cg::point_2(cg::polar_point_2(1., cur_course)) * cur_speed * dt;
    cur_course += dcourse_ * dt;

    cg::point_2 offset = cur_pos(target_pos_);
    double dist = cg::norm(offset);

    cg::point_2 loc_offset = offset * cg::rotation_2(cur_course);
    double dist_signed = loc_offset.y < 0 ? -dist : dist;

    if (dist < 1.5 * nominal_speed * dt)
    {
        cur_pos = target_pos_;
        cur_speed = 0;
    }
    else
    {
        cur_speed = filter::BreakApproachSpeed(0., dist_signed, cur_speed, nominal_speed, 10., dt, 1.1);

        double desired_course = cg::polar_point_2(offset).course;
        if (cur_speed < 0)
            desired_course = cg::norm180(180. + desired_course);
        double max_dcourse = cg::clamp(0., nominal_speed, 0., 100.)(fabs(cur_speed));
        dcourse_ = filter::BreakApproachSpeed<cg::degree180_value>(cur_course, desired_course, dcourse_, max_dcourse, 100., dt, 1.1);
    }

    max_speed = nominal_speed;


    self->set_state(state_t(cur_pos, cur_course, cur_speed));
    self->set_max_speed(max_speed);
}

follow_traj_state::follow_traj_state()
    : desired_speed_(aircraft::min_desired_velocity())
    , with_airtow_(false)
{
}

void follow_traj_state::update(model * self, double dt)
{
    cg::geo_base_2 cur_pos     = self->pos();
    double         cur_course  = self->course();
    double         cur_speed   = self->speed();
#if 0
    if(auto traj_ = self->get_trajectory())
    {
        if (true/*traj_->cur_len() < traj_->length()*/)
        {
            const double nominal_speed = with_airtow_ ? 3. : 10;
            const double cur_len = traj_->cur_len();


            traj_->set_cur_len (traj_->cur_len() + dt*desired_speed_);
            const double  tar_len = traj_->cur_len();

            desired_speed_ = 1.0; // traj_->speed_value(tar_len)?*traj_->speed_value(tar_len):nominal_speed; 
            
            decart_position target_pos;

            target_pos.pos = cg::point_3(traj_->kp_value(tar_len),0);
            geo_position gtp(target_pos, get_base());
            // self->go_to_pos(gtp.pos ,gtp.orien.get_course()); // Гы гы
            target_pos_ =  gtp.pos;
            target_course_ = gtp.orien.get_course();
//////////////////////////////////////////////////////////////////////////////////////            

            cur_pos += cg::point_2(cg::polar_point_2(1., cur_course)) * cur_speed * dt;
            cur_course += dcourse_ * dt;

            cg::point_2 offset = cur_pos(target_pos_);
            double dist = cg::norm(offset);

            cg::point_2 loc_offset = offset * cg::rotation_2(cur_course);
            double dist_signed = loc_offset.y < 0 ? -dist : dist;

            if (dist < 1.5 * nominal_speed * dt)
            {
                cur_pos = target_pos_;
                cur_speed = 0;
            }
            else
            {
                cur_speed = filter::BreakApproachSpeed(0., dist_signed, cur_speed, nominal_speed, 10., dt, 1.1);

                double desired_course = cg::polar_point_2(offset).course;
                if (cur_speed < 0)
                    desired_course = cg::norm180(180. + desired_course);
                double max_dcourse = cg::clamp(0., nominal_speed, 0., 100.)(fabs(cur_speed));
                dcourse_ = filter::BreakApproachSpeed<cg::degree180_value>(cur_course, desired_course, dcourse_, max_dcourse, 100., dt, 1.1);
            }

            self->set_state(state_t(cur_pos, cur_course, cur_speed));
            self->set_max_speed(nominal_speed);
//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Код 
            //if(cg::eq(traj_->curs_value(tar_len),traj_->curs_value(cur_len)))
            //    desired_speed_ = nominal_speed;
            //else
            //    desired_speed_ = with_airtow_ ? 3. : 5;

        }
        else
        {
#if 0

            cg::point_3 cur_pos = phys_aircraft_->get_local_position().pos;
            cg::point_3 d_pos = phys_aircraft_->get_local_position().dpos;
            cg::point_3 trg_p(traj_->kp_value(traj_->length()),0);
            d_pos.z = 0;
            if(cg::distance(trg_p,cur_pos) > 1.0 && cg::norm(d_pos) > 0.05)
            {   
                decart_position target_pos;
                target_pos.pos = trg_p;
                geo_position gp(target_pos, get_base());
                (*it).phys_aircraft_->go_to_pos(gp.pos ,gp.orien);
            }
            else
            {
                // (*it).traj.reset();
                (*it).phys_aircraft_->freeze(true);
            }

#endif

        }

    }
#else    //   new try // extern state
	FIXME(extern state);
	if(auto traj_ = self->get_trajectory())
	{

		traj_->set_cur_len (traj_->cur_len() + dt);
		const double  tar_len = traj_->cur_len();
		decart_position target_pos;

		target_pos.pos = cg::point_3(traj_->kp_value(tar_len));
		target_pos.orien = traj_->curs_value(tar_len);
		geo_position gtp(target_pos, get_base());

		self->set_state(state_t(gtp.pos, gtp.orien.get_course(), *traj_->speed_value(tar_len)));
		self->set_max_speed(100.0);

	}
#endif


}





}
