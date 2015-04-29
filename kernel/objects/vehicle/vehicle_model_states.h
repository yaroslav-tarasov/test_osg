#pragma once

#include "common/simple_route.h"
#include "geometry/curve.h"


namespace vehicle
{
struct model;

struct model_state
{      
    virtual ~model_state() {}

    virtual void update(model * self, double dt) = 0;
    virtual bool end() const {return false;}
};

typedef polymorph_ptr<model_state> model_state_ptr;

//////////////////////////////////////////////////////////////////////////
struct follow_route_state : model_state
{
    follow_route_state(simple_route::info_ptr route);

    void update(model * self, double dt);

private:
    simple_route::info_ptr route_;
    double dcourse_;
};

typedef polymorph_ptr<follow_route_state> follow_route_state_ptr;

struct follow_curve_state : model_state
{
    follow_curve_state(cg::geo_curve_2 const& route, double end_course, bool with_airtow);

    void update(model * self, double dt);
    bool end() const {return end_;}

private:
    cg::geo_curve_2 route_;
    double dcourse_;
    bool with_airtow_;
    double end_course_;
    bool end_;
};


struct follow_traj_state :model_state
{
    follow_traj_state();
    void update(model * self, double dt);

private:
    double                desired_velocity_;
};


struct go_to_pos_state : model_state
{
    go_to_pos_state(cg::geo_point_2 const& pos, optional<double> course, bool with_airtow);

    void update(model * self, double dt);

private:
    bool with_airtow_;
    cg::geo_point_2 target_pos_;
    optional<double> target_course_;
    double dcourse_;
};
} // vehicle