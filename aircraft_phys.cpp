#include "stdafx.h"
#include "aircraft_phys.h"


namespace phys
{
    aircraft::aircraft(/*compound_sensor_t const* s,*/ params_t const& params, decart_position const& pos)
        : rigid_body_user_info_t(rb_aircraft)
    {

    }

void aircraft::updateAction( btCollisionWorld* collisionWorld, btScalar deltaTimeStep)
{

}

void aircraft::debugDraw(btIDebugDraw* debugDrawer)
{

}

}
