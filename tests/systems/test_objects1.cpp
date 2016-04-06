#include "stdafx.h"
#include "kernel/systems_fwd.h"

#include "kernel/systems/systems_base.h"
#include "kernel/systems/fake_system.h"
#include "kernel/object_class.h"

FIXME(Это что за нафиг нужно  для object_creators )
#include "common/simple_route.h"
#include "objects/flock_manager/flock_manager_common.h"
#include "common/flock_manager.h"
#include "objects/aerostat/aerostat_common.h"
#include "common/aerostat.h"
#include "vehicle/vehicle_common.h"
#include "airport/airport_common.h"
#include "aircraft/aircraft_common.h"

#include "objects/human/human_common.h"
#include "common/human.h"

#include "object_creators.h"

#include "common/test_msgs.h"

#include "factory_systems.h"

#include "utils/krv_import.h"


using namespace kernel;

namespace 
{
    inline std::string data_file(std::string icao_code)
    {
        if (icao_code == "UUEE")
            return "log_sochi_4.txt";
        else if (icao_code == "URSS")
            return "log_sochi_4.txt";
        else if (icao_code == "UMMS" || icao_code == "UMMS")
            return "log_minsk.txt";

        return "";
    }

void create_objects(const std::string & airport)
{
    const std::string icao_code = airport; 

    krv::data_getter              _krv_data_getter(data_file(icao_code));
    
    high_res_timer hr_timer;

    // Только получение без контроля  
    kernel::system_ptr _csys = get_systems()->get_control_sys();
    
    {
        airport::settings_t as;
        as.icao_code = icao_code;
        auto obj_airport = airport::create(dynamic_cast<fake_objects_factory*>(kernel::fake_objects_factory_ptr(_csys).get()),as);
    }
    
    force_log fl5;       
    LOG_ODS_MSG( "create_objects(const std::string& airport): airport::settings_t " << hr_timer.set_point() << "\n");

#if 0
	{
		auto obj_camera = camera::create(dynamic_cast<fake_objects_factory*>(kernel::fake_objects_factory_ptr(_csys).get()), std::string("camera 0"));
	}
#endif

    if(false)
    {
        std::string class_name = "aircraft";
        std::string unique_name = "aircraft_0";
        std::vector<object_class_ptr> const &classes = kernel::fake_objects_factory_ptr(_csys)->object_classes() ;

        kernel::object_class_ptr class_ptr ;

        for (auto it = classes.begin(), end = classes.end(); it != end; ++it)
        {
            if (class_name == (*it)->name())
                class_ptr = *it ;
            std::string n = (*it)->name();
        }

        auto obj = kernel::fake_objects_factory_ptr(_csys)->create_object(class_ptr, unique_name); 
        nodes_management::manager_ptr manager = find_first_child<nodes_management::manager_ptr>(obj);
        manager->set_model(aircraft::get_model("A319"));
    }

    {
        cg::geo_point_3 apos(0.0,0.0005,0.0);
        aircraft::settings_t as;
        as.kind = "A319";
        geo_position agp(apos,quaternion(cpr(30,0,0)));
        //auto obj_aircraft = aircraft::create(dynamic_cast<fake_objects_factory*>(kernel::fake_objects_factory_ptr(_csys).get()),as,agp);
    }
    {

        cg::geo_point_3 apos(-0.0006,-0.0005,0.0);
        aircraft::settings_t as;
        as.kind = "SB20";
        geo_position agp(apos,quaternion(cpr(0,0,0)));
        //auto obj_aircraft = aircraft::create(dynamic_cast<fake_objects_factory*>(kernel::fake_objects_factory_ptr(_csys).get()),as,agp);
    }

    {
        cg::geo_point_3 apos(0.0,0.0009,0.0);
        aircraft::settings_t as;
        as.kind = "A321";
        geo_position agp(apos,quaternion(cpr(60,0,0)));
        //auto obj_aircraft2 = aircraft::create(dynamic_cast<fake_objects_factory*>(kernel::fake_objects_factory_ptr(_csys).get()),as,agp);
    }

    {
        cg::geo_point_3 apos(0.0,0.0018,0.0);
        aircraft::settings_t as;
        as.kind = "B737";
        geo_position agp(apos,quaternion(cpr(60,0,0)));
        //auto obj_aircraft2 = aircraft::create(dynamic_cast<fake_objects_factory*>(kernel::fake_objects_factory_ptr(_csys).get()),as,agp);
    }

    {
        cg::geo_point_3 apos(0.0,0.0027,0.0);
        aircraft::settings_t as;
        as.kind = "B744";
        geo_position agp(apos,quaternion(cpr(60,0,0)));
        //auto obj_aircraft2 = aircraft::create(dynamic_cast<fake_objects_factory*>(kernel::fake_objects_factory_ptr(_csys).get()),as,agp);
    }

    {
        cg::geo_point_3 apos(0.0,0.0036,0.0);
        aircraft::settings_t as;
        as.kind = "B763";
        geo_position agp(apos,quaternion(cpr(60,0,0)));
        //auto obj_aircraft2 = aircraft::create(dynamic_cast<fake_objects_factory*>(kernel::fake_objects_factory_ptr(_csys).get()),as,agp);
    }

    {
        // cg::geo_point_3 apos(0.0,-0.0005/*0.0045*/,0.0);
        cg::point_3 vpos(350,650,0);
        decart_position target_pos(vpos,cg::quaternion(cg::cpr(30, 0, 0)));
        geo_position agp(target_pos, ::get_base());

        aircraft::settings_t as;
        as.kind = "AN26";//"A333";
        //geo_position agp(apos,quaternion(cpr(60,0,0)));
        //auto obj_aircraft2 = aircraft::create(dynamic_cast<fake_objects_factory*>(kernel::fake_objects_factory_ptr(_csys).get()),as,agp);
    }

    {
        // cg::geo_point_3 apos(0.0,-0.0005/*0.0045*/,0.0);
        cg::point_3 vpos(300,600,0);
        decart_position target_pos(vpos,cg::quaternion(cg::cpr(30, 0, 0)));
        geo_position agp(target_pos, ::get_base());

        aircraft::settings_t as;
        as.kind = "A319";//"A333";
        //geo_position agp(apos,quaternion(cpr(60,0,0)));
//      auto obj_aircraft2 = aircraft::create(dynamic_cast<fake_objects_factory*>(kernel::fake_objects_factory_ptr(_csys).get()),as,agp);
    }

    force_log fl2;       
    LOG_ODS_MSG( "create_objects(const std::string& airport): aircraft::create " << hr_timer.set_point() << "\n");

    {
        cg::point_3 vpos(250,650,0);
        decart_position target_pos(vpos,cg::quaternion(cg::cpr(30, 0, 0)));
        geo_position agp(target_pos, ::get_base());

        aircraft::settings_t as;
        as.kind = "KA27";
        //auto obj_aircraft2 = aircraft::create(dynamic_cast<fake_objects_factory*>(kernel::fake_objects_factory_ptr(_csys).get()),as,agp);
    }

    fms::trajectory::keypoints_t  kpts;
    fms::trajectory::curses_t      crs;
    fms::trajectory::speed_t vls ;
	


    const unsigned start_idx = 400;
	double tlength = 0;
	if(_krv_data_getter.kp_.size()>0)
	{
		cg::point_2 prev(_krv_data_getter.kd_[start_idx].x,_krv_data_getter.kd_[start_idx].y);

		for(auto it = _krv_data_getter.kd_.begin() + start_idx; it!= _krv_data_getter.kd_.end();++it )
		{
			auto p = cg::point_2(it->x,it->y);
			auto dist = cg::distance(prev,p);
			tlength += dist;
			crs.insert(std::make_pair(tlength,cpr(it->fiw,it->tg, it->kr )));
			kpts.insert(std::make_pair(tlength,cg::point_3(p,it->h)));

			auto pit = std::prev(it);
			if(pit!=it)
				vls.insert(std::make_pair(tlength,dist/(it->time - pit->time)/* + 20*/));
			// vel.push_back(dist/(it->time - pit->time));
			prev = p;
		}
	}

    force_log fl3;       
    LOG_ODS_MSG( "create_objects(const std::string& airport): fms::trajectory::velocities " << hr_timer.set_point() << "\n");
	if(_krv_data_getter.kp_.size()>0)
    {
        cg::point_3 vpos(_krv_data_getter.kp_[start_idx]);
        decart_position target_pos(vpos,cpr(/*180 -*/ _krv_data_getter.kd_[start_idx].fiw,0,0));
        geo_position agp(target_pos, ::get_base());

        agp.orien = cpr( _krv_data_getter.kd_[start_idx].fiw /*- 45*/,0,0);

        aircraft::settings_t as;
        as.kind = "A319";
//      auto obj_aircraft = aircraft::create(dynamic_cast<fake_objects_factory*>(kernel::fake_objects_factory_ptr(_csys).get()),as,agp);
//      aircraft::int_control_ptr(obj_aircraft)->set_trajectory(fms::trajectory::create(kpts,crs,vls));
    }
	
	if(_krv_data_getter.kp_.size()>0)
    {
        // cg::geo_point_3 apos(0.0,-0.0005/*0.0045*/,0.0);
        cg::point_3 vpos(350,650,0);
        decart_position target_pos(vpos,cg::quaternion(cg::cpr(_krv_data_getter.kd_[start_idx].fiw , 0, 0)));
        //geo_position agp(target_pos, ::get_base());

        aircraft::settings_t as;
        as.kind = "A319";//"A333";

		cg::geo_point_3 apos(0.00045,0.00087,0.0);
		geo_position agp(apos,quaternion(cpr(30,0,0)));

#if 0
        //geo_position agp(apos,quaternion(cpr(60,0,0)));
        auto obj_aircraft2 = aircraft_physless::create(dynamic_cast<fake_objects_factory*>(kernel::fake_objects_factory_ptr(_csys).get()),as,agp);
//      aircraft::int_control_ptr(obj_aircraft2)->set_trajectory(fms::trajectory::create(kpts,crs,vls));
#endif
    }


		vehicle::settings_t vs;
		vs.model = "buksir";//"niva_chevrolet";//

		//cg::point_3 vpos(330,750,00);
		//cg::point_3 vpos(572,032,0);
		//decart_position target_pos(vpos,cg::quaternion(cg::cpr(30, 0, 0)));
		//geo_position vgp(target_pos, ::get_base());

		cg::geo_point_3 vpos(0.0006,0.0009,0.0);
		geo_position vgp(vpos,quaternion(cpr(30 + 180,0,0)));

#if 0
		auto obj_vehicle = vehicle::create(dynamic_cast<fake_objects_factory*>(kernel::fake_objects_factory_ptr(_csys).get()),vs,vgp);
#endif
		force_log fl9;       
		LOG_ODS_MSG( "create_objects(const std::string& airport): vehicle::create " << hr_timer.set_point() << "\n");


	{

	vehicle::settings_t vs;
	vs.model = "buksir";//"niva_chevrolet";//
	cg::geo_point_3 vpos(0.00065,0.00095,0.0);
	geo_position vgp(vpos,quaternion(cpr(30,0,0)));

	//auto obj_vehicle = vehicle::create(dynamic_cast<fake_objects_factory*>(kernel::fake_objects_factory_ptr(_csys).get()),vs,vgp);

	}


	{
		vehicle::settings_t vs;
		vs.model = "buksir";//"niva_chevrolet";//
		cg::geo_point_3 vpos(0.00055,0.0009,0.0);
		geo_position vgp(vpos,quaternion(cpr(30,0,0)));

	//	auto obj_vehicle = vehicle::create(dynamic_cast<fake_objects_factory*>(kernel::fake_objects_factory_ptr(_csys).get()),vs,vgp);
	}


	{
		vehicle::settings_t vs;
		vs.model = "buksir";//"niva_chevrolet";//

		cg::geo_point_3 vpos(0.0005,0.0009,0.0);
		geo_position vgp(vpos,quaternion(cpr(30,0,0)));

	//	auto obj_vehicle = vehicle::create(dynamic_cast<fake_objects_factory*>(kernel::fake_objects_factory_ptr(_csys).get()),vs,vgp);
	}

    //const kernel::object_collection  *  col = dynamic_cast<kernel::object_collection *>(_csys.get());
    //auto vvv = find_object<vehicle::control_ptr>(col,"vehicle 0");
    //auto nm = find_first_child<nodes_management::manager_ptr>(vvv);
    //uint32_t nm_id = kernel::object_info_ptr(nm)->object_id();
    //uint32_t vv_id = kernel::object_info_ptr(vvv)->object_id();

    // auto sr_obj = create_object(_csys,"simple_route","simple_route_0");

    simple_route::settings_t srs;
    srs.speed = 6;
    auto sr_obj = simple_route::create(dynamic_cast<fake_objects_factory*>(kernel::fake_objects_factory_ptr(_csys).get()),srs,vgp.pos);

    force_log fl6;       
    LOG_ODS_MSG( "create_objects(const std::string& airport): simple_route::create " << hr_timer.set_point() << "\n");


#if 0
    {
        flock::manager::settings_t vs;
        // vs.model = "crow";
        cg::geo_point_3 vpos(0.00055,0.0009,300.0);
        geo_position vgp(vpos,quaternion(cpr(30,0,0)));

        auto obj_flock_manager = flock::manager::create(dynamic_cast<fake_objects_factory*>(kernel::fake_objects_factory_ptr(_csys).get()),vs,vgp);
    }
#endif

#if 0
	{
		aerostat::settings_t vs;
	    vs.model = "aerostat";
		cg::geo_point_3 vpos(0.00055,0.0009,300.0);
		geo_position vgp(vpos,quaternion(cpr(30,0,0)));

		auto obj_aerostat = aerostat::create(dynamic_cast<fake_objects_factory*>(kernel::fake_objects_factory_ptr(_csys).get()),vs,vgp);
	}
#endif

#if 0
    {
        human::settings_t vs;
        vs.model = "human";
        cg::geo_point_3 vpos(0.00055,/*0.0009*/0.0,2.0);
        geo_position vgp(vpos, point_3(0,10,0), quaternion(cpr(30,0,0)), point_3());

        auto obj_aerostat = human::create(dynamic_cast<fake_objects_factory*>(kernel::fake_objects_factory_ptr(_csys).get()),vs,vgp);
    }
#endif
}

using namespace net_layer::msg;

inline object_info_ptr create_aircraft(kernel::system* csys,create const& msg)
{
    // cg::geo_point_3 apos(msg.lat,msg.lon,0.0);

    decart_position target_pos;

    target_pos.pos   = msg.pos;
    target_pos.orien = msg.orien;
    geo_position agp(target_pos, get_base());

    aircraft::settings_t as;
    as.kind = msg.model_name; // "A319"; //
    // geo_position agp(apos,quaternion(cpr(msg.course,0,0)));
    return  aircraft::create(dynamic_cast<fake_objects_factory*>(csys),as,agp);
}

inline object_info_ptr create_helicopter_phl(kernel::system* csys,create const& msg)
{
    // cg::geo_point_3 apos(msg.lat,msg.lon,0.0);

    decart_position target_pos;

    target_pos.pos   = msg.pos;
    target_pos.orien = msg.orien;
    geo_position agp(target_pos, get_base());

    aircraft::settings_t as;
    as.kind = msg.model_name;  
    as.custom_label = msg.custom_label;

    return  helicopter_physless::create(dynamic_cast<fake_objects_factory*>(csys),as,agp);
}

inline object_info_ptr create_aircraft_phl(kernel::system* csys,create const& msg)
{
    // cg::geo_point_3 apos(msg.lat,msg.lon,0.0);

    decart_position target_pos;

    target_pos.pos   = msg.pos;
    target_pos.orien = msg.orien;
    geo_position agp(target_pos, get_base());

    aircraft::settings_t as;
    as.kind = msg.model_name; // "A319"; //
    as.custom_label = msg.custom_label;

    // geo_position agp(apos,quaternion(cpr(msg.course,0,0)));
    return  aircraft_physless::create(dynamic_cast<fake_objects_factory*>(csys),as,agp);
}

inline object_info_ptr create_vehicle(kernel::system* csys,create const& msg)
{
    decart_position target_pos;

    target_pos.pos   = msg.pos;
    target_pos.orien = msg.orien;
    geo_position vgp(target_pos, get_base());

    vehicle::settings_t vs;
    vs.model        = msg.model_name;
    vs.custom_label = msg.custom_label;

    return  vehicle::create(dynamic_cast<fake_objects_factory*>(csys),vs,vgp);
}

inline object_info_ptr create_flock_of_birds(kernel::system* csys, create const& msg)
{
    decart_position target_pos;

    target_pos.pos   = msg.pos;
    target_pos.orien = msg.orien;
    geo_position vgp(target_pos, get_base());
    
    flock::manager::settings_t vs;
    vs.model        = "crow";
    vs._childAmount = msg.num_instances;

    return flock::manager::create(dynamic_cast<fake_objects_factory*>(csys),vs,vgp);
}

inline object_info_ptr create_character(kernel::system* csys, create const& msg)
{
    decart_position target_pos;

    target_pos.pos   = msg.pos;
    target_pos.orien = msg.orien;
    geo_position vgp(target_pos, get_base());

    human::settings_t vs;
    vs.model = msg.model_name;

    return human::create(dynamic_cast<fake_objects_factory*>(csys),vs,vgp);
}
 


inline object_info_ptr create_aerostat(kernel::system* csys, create const& msg)
{
	decart_position target_pos;

	target_pos.pos   = msg.pos;
	target_pos.orien = msg.orien;
	geo_position vgp(target_pos, get_base());

	aerostat::settings_t ms;
	ms.model = "aerostat";

	return aerostat::create(dynamic_cast<fake_objects_factory*>(csys),ms,vgp);
}

inline object_info_ptr create_camera(kernel::system* csys, create const& msg)
{
	return camera::create(dynamic_cast<fake_objects_factory*>(csys), msg.model_name);
}

object_info_ptr create_object( kernel::system* csys, create const& msg)
{
    if(msg.object_kind & ok_vehicle)
        return create_vehicle(csys, msg);
    else if ( msg.object_kind == ok_flock_of_birds)
        return create_flock_of_birds(csys, msg);
    else if ( msg.object_kind ==ok_human)
        return create_character(csys, msg);
    else if ( msg.object_kind == ok_helicopter)
        return create_helicopter_phl(csys, msg); 
    else if( msg.object_kind == ok_camera)
        return create_camera(csys, msg);
    else
        return create_aircraft_phl(csys, msg);  // FIXME вместо чекера можно создать какой-нибудь более дурной объект

}



}

AUTO_REG(create_objects)
AUTO_REG_NAME(create_object, create_object)
