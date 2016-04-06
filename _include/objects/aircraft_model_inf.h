#pragma once

// Вариация продолжение для aircraft.h
namespace aircraft
{

    struct int_control
    {
         virtual ~int_control(){}
         virtual void                 update(double dt)     =0;
         virtual fms::trajectory_ptr  get_trajectory()      =0;
         virtual decart_position      get_local_position()  =0; 
         virtual void                 set_trajectory(fms::trajectory_ptr  traj)=0;
    };

}