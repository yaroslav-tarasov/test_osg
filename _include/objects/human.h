#pragma once

namespace human
{

    struct control
    {
         virtual ~control(){};
         virtual void goto_pos(geo_point_2 pos,double course)=0;
         virtual void follow_route(const std::string& name)=0;
        
         FIXME(Надо сократить до follow_trajectory)
         virtual void follow_trajectory(std::string const& /*route*/)=0;

         virtual decart_position      get_local_position() const=0;
         virtual fms::trajectory_ptr  get_trajectory()=0;
         virtual void set_trajectory(fms::trajectory_ptr  traj)=0;
    };

}



