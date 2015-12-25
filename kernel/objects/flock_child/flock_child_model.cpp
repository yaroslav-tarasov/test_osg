#include "stdafx.h"
#include "precompiled_objects.h"

#include "flock_child_model.h"
//#include "common/collect_collision.h"
#include "phys/sensor.h"

namespace flock
{

namespace child
{

object_info_ptr model::create(kernel::object_create_t const& oc, dict_copt dict)
{
    return object_info_ptr(new model(oc,dict));
}

AUTO_REG_NAME(flock_child_model, model::create);

model::model( kernel::object_create_t const& oc, dict_copt dict )
    : view                  (oc, dict)
    , phys_object_model_base(collection_)
    , sys_(dynamic_cast<model_system *>(oc.sys))
    , nodes_manager_        (find_first_child<nodes_management::manager_ptr>(this))
    , _speed                (10.0)
{
    settings_._avoidValue = rnd_.random_range(.2, .4/*.3, .1*/);
    create_phys();
}

void model::update(double time)
{
    view::update(time);
	
	double dt = time - (last_update_ ? *last_update_ : 0);
    if (!cg::eq_zero(dt))
    {

        //Soar Timeout - Limits how long a bird can soar
        if(_soar && _spawner->settings()._soarMaxTime > 0){ 		
            if(_soarTimer >_spawner->settings()._soarMaxTime){
                flap();
                _soarTimer = 0;
            }else {
                _soarTimer+=dt;
            }
        }


#if 0
        update_model(time, dt);

        if (!manual_controls_)
#endif
            sync_phys(dt);
#if 0
        else
        {
            cg::geo_base_3 base = phys_->get_base(*phys_zone_); 
            decart_position cur_pos = phys_vehicle_->get_position();
            geo_position cur_glb_pos(cur_pos, base);
            set_state(state_t(cur_glb_pos.pos, cur_pos.orien.get_course(), 0)); 
        }
#endif


        sync_nodes_manager(dt);

    }

#if 0
		if(!_landingSpotted && (transform.position - _wayPoint).magnitude < _spawner._waypointDistance+_stuckCounter){
			Wander(0);	//create a new waypoint
			_stuckCounter=0;
		}else{
			_stuckCounter+=dt;
		}


		if(_targetSpeed > -1){
			var rotation = Quaternion.LookRotation(_wayPoint - transform.position);
			transform.rotation = Quaternion.Slerp(transform.rotation, rotation, Time.deltaTime * _damping);
		}
		if(_spawner._childTriggerPos){
			if((transform.position - _spawner.transform.position).magnitude < 1){
				_spawner.randomPosition();
			}
		}
		_speed = Mathf.Lerp(_speed, _targetSpeed, _lerpCounter * Time.deltaTime *.05);
		_lerpCounter++;
		//Position forward based on object rotation
		if(_move){
			transform.position += transform.TransformDirection(Vector3.forward)*_speed*Time.deltaTime;
			//Avoidance
			if(_spawner._birdAvoid){	
				var hit : RaycastHit;
				if (Physics.Raycast(transform.position, transform.forward+(transform.right*_avoidValue), hit, _avoidDistance)){
					if(!hit.transform.gameObject.GetComponent(FlockController)){
						transform.rotation.eulerAngles.y -= _spawner._birdAvoidHorizontalForce*Time.deltaTime*_damping;
					}
				}else if (Physics.Raycast(transform.position, transform.forward+(transform.right*-_avoidValue), hit, _avoidDistance)){
					if(!hit.transform.gameObject.GetComponent(FlockController)){
						transform.rotation.eulerAngles.y += _spawner._birdAvoidHorizontalForce*Time.deltaTime*_damping;
					}
				}

				if (_spawner._birdAvoidDown && Physics.Raycast(transform.position, -Vector3.up, hit, _avoidDistance)){
					if(!hit.transform.gameObject.GetComponent(FlockController)){
						transform.rotation.eulerAngles.x -= _spawner._birdAvoidVerticalForce*Time.deltaTime*_damping;
					}
				}else if (_spawner._birdAvoidUp && Physics.Raycast(transform.position, Vector3.up, hit, _avoidDistance)){
					if(!hit.transform.gameObject.GetComponent(FlockController)){
						transform.rotation.eulerAngles.x += _spawner._birdAvoidVerticalForce*Time.deltaTime*_damping;
					}
				}
			}
		}
		//Counteract Pitch Rotation When Flying Upwards
		if((_soar && _spawner._flatSoar|| _spawner._flatFly && !_soar)&& _wayPoint.y > transform.position.y||_flatFlyDown)
			_model.transform.localEulerAngles.x = Mathf.LerpAngle(_model.transform.localEulerAngles.x, -transform.localEulerAngles.x, _lerpCounter * Time.deltaTime * .25);
		else
			_model.transform.localEulerAngles.x = Mathf.LerpAngle(_model.transform.localEulerAngles.x, 0, _lerpCounter * Time.deltaTime * .25);

		//btVector3 btFrom(camPos.x, camPos.y, camPos.z);
		//btVector3 btTo(camPos.x, -5000.0f, camPos.z);
		//btCollisionWorld::ClosestRayResultCallback res(btFrom, btTo);

		//Base::getSingletonPtr()->m_btWorld->rayTest(btFrom, btTo, res); // m_btWorld is btDiscreteDynamicsWorld

		//if(res.hasHit()){
		//	printf("Collision at: <%.2f, %.2f, %.2f>\n", res.m_hitPointWorld.getX(), res.m_hitPointWorld.getY(), res.m_hitPointWorld.getZ());
		//}

#endif

    last_update_ = time;

}

void model::on_child_removing(object_info_ptr child)
{
    view::on_child_removing(child);

    if (nodes_manager_ == child)
        nodes_manager_.reset();
}


void model::create_phys()
{
    if (!phys_ || !root_)
        return;

    phys_zone_ = phys_->get_zone(cg::geo_point_3(pos(), 0));
    if (!phys_zone_)
        return;

    cg::geo_base_3 base = phys_->get_base(*phys_zone_);

    //phys::sensor_ptr s = collect_collision(nodes_manager_, body_node_);
    phys::compound_sensor_ptr s = phys::flock::fill_cs(nodes_manager_); 
    decart_position p(/*veh_transform.translation()*/base(state_.pos), /*cg::quaternion(veh_transform.rotation().cpr())*/state_.orien);
    
    phys::flock::params_t  params;
    params.mass = 1;
    phys_flock_ = phys_->get_system(*phys_zone_)->create_flock_child(params, s, p);

}

void model::sync_phys(double dt)
{
    if (!phys_flock_ || !phys_)
        return;

    //        double const max_break_accel = aerotow_ ? 2 : 20;
    double const max_accel = 15;
    //        double const smooth_factor = 5.;

    cg::geo_base_3 base = phys_->get_base(*phys_zone_);

    decart_position cur_pos = phys_flock_->get_position();

    geo_position cur_glb_pos(cur_pos, base);
    double cur_speed = cg::norm(cur_pos.dpos);
    double cur_course = cur_pos.orien.cpr().course;

    point_3 forward_dir = cg::normalized_safe(cur_pos.orien.rotate_vector(point_3(0, 1, 0))) ;
    point_3 right_dir   = cg::normalized_safe(cur_pos.orien.rotate_vector(point_3(1, 0, 0))) ;
    point_3 up_dir      = cg::normalized_safe(cur_pos.orien.rotate_vector(point_3(0, 0, 1))) ;

    // transform.position += transform.TransformDirection(Vector3.forward)*_speed*Time.deltaTime;
    //cur_pos.pos    = cur_pos.pos;// +  up_dir * _speed  * dt;
    //cur_pos.dpos   = right_dir * _speed * dt;
    //cur_pos.orien  = cpr(0);
    //cur_pos.omega  = point_3(0.0,0.0,0.0);

    // phys::flock::control_ptr(phys_flock_)->set_position(cur_pos);
    phys::flock::control_ptr(phys_flock_)->set_linear_velocity(-forward_dir * _speed * 10);
    phys::flock::control_ptr(phys_flock_)->set_angular_velocity(point_3(0.0,0.0,0.1));
}

void model::sync_nodes_manager( double /*dt*/ )
{
    if (phys_flock_ && root_)
    {
        cg::geo_base_3 base = phys_->get_base(*phys_zone_);
        decart_position bodypos = phys_flock_->get_position();
        decart_position root_pos = bodypos /** body_transform_inv_*/;// FIXME Модельно зависимое решение 

        geo_position pos(root_pos, base);

        // FIXME Глобальные локальные преобразования 
        nodes_management::node_position root_node_pos = root_->position();
        root_node_pos.global().pos = root_next_pos_;
        root_node_pos.global().dpos = cg::geo_base_3(root_next_pos_)(pos.pos) / (sys_->calc_step());

        root_node_pos.global().orien = root_next_orien_;
        root_node_pos.global().omega = cg::get_rotate_quaternion(root_node_pos.global().orien, pos.orien).rot_axis().omega() / (sys_->calc_step());

        // nodes_management::node_position rnp = local_position(0,0,cg::geo_base_3(get_base())(root_node_pos.global().pos),root_node_pos.global().orien);
        root_->set_position(root_node_pos);

        root_next_pos_ = pos.pos;
        root_next_orien_ = pos.orien;

        //geo_position body_pos(phys_vehicle_->get_position() * body_transform_inv_, base);
    }
}

void model::flap()
{

}

}

} // end of flock
