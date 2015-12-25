#pragma once


#include "flock_manager_common.h"
#include "common/flock_child.h"
#include "common/flock_manager.h"

namespace flock
{

	namespace manager
	{

        struct manager_data
        {
            manager_data()
            {
            }

            manager_data(settings_t const& settings, state_t const& state)
                : settings_(settings)
                , state_   (state  )
            {
            }

        protected:
            settings_t settings_;
            state_t    state_;
#if 0


            var _slowSpawn:boolean;					// Birds will not be instantiated all at once at start
            var _spawnSphere:float = 3;				// Range around the spawner waypoints will created
            var _spawnSphereHeight:float = 1.5;		// Height of the spawn sphere

            var _soarFrequency:float = 0;			// How often soar is initiated 1 = always 0 = never
            var _soarAnimation:String="Soar";		// Animation -required- for soar functionality
            var _flapAnimation:String="Flap";		//
            var _idleAnimation:String="Idle";		// Animation -required- for sitting idle functionality
            var _diveValue:float = 7;				// Dive depth
            var _diveFrequency:float = 0.5;			// How often dive 1 = always 0 = never
            var _minDamping:float = 1;				// Rotation tween damping, lower number = smooth/slow rotation (if this get stuck in a loop, increase this value)
            var _maxDamping:float = 2;
            var _waypointDistance:float = 1;		// How close this can get to waypoint before creating a new waypoint (also fixes stuck in a loop)
            var _randomPositionTimer:float = 10;	// *** 
            var _positionSphere:int = 25;			// If _randomPositionTimer is bigger than zero the controller will be moved to a random position within this sphere
            var _positionSphereHeight = 5;			// Overides height of sphere for more controll
            var _childTriggerPos:boolean;			// Runs the random position function when a child reaches the controller
            var _forceChildWaypoints:boolean;		// Forces all children to change waypoints when this changes position
            var _forcedRandomDelay:float = 1.5;		// Random delay added before forcing new waypoint
            var _flatFly:boolean;
            var _flatSoar:boolean;					
            var _flockAvoid:boolean;				// Enable physics on flock to avoid creating waypoint inside other objects
            var _birdAvoid:boolean;					// Avoid colliders left and right
            var _birdAvoidHorizontalForce:int = 1000;
            var _birdAvoidDown:boolean;				// Avoid colliders below
            var _birdAvoidUp:boolean;				// Avoid colliders above bird
            var _birdAvoidVerticalForce:int = 300;
            var _flockColliderSize:float = 1;		// Collider size of the flock
            var _birdAvoidDistanceMax:float = 5;		//
            var _birdAvoidDistanceMin:float = 5;		// 
            var _soarMaxTime:float;					// Stops soaring after x seconds, use to avoid birds soaring for too long

#endif

            REFL_INNER(manager_data)
                REFL_ENTRY(settings_)
                REFL_ENTRY(state_)
           REFL_END()
        };


		struct view
			: base_view_presentation
			, obj_data_holder<manager_data>
			, info
		{
			static object_info_ptr create(kernel::object_create_t const& oc, dict_copt dict);

		protected:
			view(kernel::object_create_t const& oc, dict_copt dict);

			// base_view
		protected:
			void on_object_created(object_info_ptr object)    override;
			void on_object_destroying(object_info_ptr object) override;
			// info
			const settings_t& settings()                 const override;
		protected:
			std::set<child::info_ptr>				       	roamers_;

		};


	}

}
