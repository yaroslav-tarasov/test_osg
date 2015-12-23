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
    , nodes_manager_        (find_first_child<nodes_management::manager_ptr>(this))
    , _spawner              (find_first_object<manager::info_ptr>(collection_))
{
	simplerandgen  rnd(static_cast<unsigned>(time(nullptr)));
    settings_._avoidValue = rnd.random_range(.3, .1);
    create_phys();
}

void model::update(double time)
{
    view::update(time);
	
	double dt = time - (last_update_ ? *last_update_ : 0);

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
}

void model::flap()
{

}

}

} // end of flock
