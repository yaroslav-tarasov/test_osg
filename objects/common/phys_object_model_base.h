#pragma once

#include "common/phys_sys.h"

//! базовый класс для объектов с физическими системами
struct phys_object_model_base
{
    phys_object_model_base(object_collection * collection)
        : phys_                 (find_first_object<phys::control_ptr>(collection))
        
        , zone_created_conn_    (phys_->subscribe_zone_created  (boost::bind(&phys_object_model_base::on_zone_created  , this, _1)))
        , zone_destroyed_conn_  (phys_->subscribe_zone_destroyed(boost::bind(&phys_object_model_base::on_zone_destroyed, this, _1)))
    {
    }

    virtual void on_zone_created  ( size_t /*id*/ ) {}
    virtual void on_zone_destroyed( size_t /*id*/ ) {}

protected:
    phys::control_ptr phys_;

private:
    scoped_connection zone_created_conn_  ;
    scoped_connection zone_destroyed_conn_;
};