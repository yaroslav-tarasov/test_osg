// network.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "ipc/ipc.h"

#include "common/cmd_line.h"

#include "async_services/async_services.h"
#include "network/msg_dispatcher.h"
#include "logger/logger.hpp"

#include "common/test_msgs.h"
#include "common/test_msgs2.h"

#include "utils/krv_import.h"

#include "kernel/systems.h"
#include "reflection/proc/prop_tree.h"

#include  "configurer.h"

#include "av/avEnvironmentParams.h"

using network::endpoint;
using network::async_acceptor;
using network::async_connector;
using network::tcp_socket;
using network::error_code;

using network::msg_dispatcher;

typedef boost::system::error_code       error_code_t;

using namespace net_layer::msg;
using network::tcp_fragment_wrapper;

namespace
{
    boost::asio::io_service* __main_srvc__ = 0;

    void tcp_error(error_code const& err)
    {
        LogError("Error happened: " << err);
        __main_srvc__->stop();
    }

    std::string              g_icao_code = "URSS";
}


inline fms::trajectory_ptr fill_trajectory (const krv::data_getter& kdg)
{

    fms::trajectory::keypoints_t  kpts;
    fms::trajectory::curses_t      crs;
    fms::trajectory::speed_t       vls;

    const unsigned start_idx = 0;
    cg::point_2 prev(kdg.kd_[start_idx].x,kdg.kd_[start_idx].y);
    double tlength = 0;
    for(auto it = kdg.kd_.begin() + start_idx; it!= kdg.kd_.end();++it )
    {
        auto p = cg::point_2(it->x,it->y);
        auto dist = cg::distance(prev,p);
        tlength += dist;
        crs.insert (std::make_pair(it->time,cpr(it->fiw,it->tg, it->kr )));
        kpts.insert(std::make_pair(it->time,cg::point_3(p,it->h)));

        if(it != kdg.kd_.begin())
            vls.insert(std::make_pair(it->time,dist/(it->time - std::prev(it)->time)));

        prev = p;
    }

    return fms::trajectory::create(kpts,crs,vls);
}

inline fms::trajectory_ptr fill_trajectory ()
{

    fms::trajectory::keypoints_t  kpts;
    fms::trajectory::curses_t      crs;
    fms::trajectory::speed_t       vls;

    const unsigned start_idx = 0;

    crs.insert (std::make_pair(26.0,cpr(20)));
    kpts.insert(std::make_pair(26.0,point_3f(48.89,411.65,28.50)));

    crs.insert (std::make_pair(59.0,cpr(180)));
    kpts.insert(std::make_pair(59.0,point_3f(300.89,411.65,28.50)));

    return fms::trajectory::create(kpts,crs,vls);
}


#define ADD_EVENT(time, msg)                                   \
  msgs_.insert(make_pair(time,                                 \
       std::move(network::wrap_msg(msg))                       \
    ));                                                        

namespace
{

    const  double traj_offset = 20.;
    const  double vehicle_prediction = 0;
    const  double factor = 1.0; 

} 

inline optional<geo_point_3> kta_position( string icao_code )
{
	if (icao_code == "UUEE")
		return geo_point_3(55.9724 , 37.4131 , 0.);
	else if (icao_code == "URSS")
		return geo_point_3(43.44444, 39.94694, 0.);
	else if (icao_code == "UHWW")
		return geo_point_3(43.397928957854155, 132.14877916666666, 17.);
	else if (icao_code == "UUOB")
		return geo_point_3(50. + 38. / 60. + 38. / 3600., 36. + 35. / 60. + 24. / 3600., 0.);

	return optional<geo_point_3>();
}


struct client
{

    DECL_LOGGER("visa_control");
	
    typedef std::vector<std::pair<endpoint, bool> > endpoints;
	
	struct connect_helper
	{
		connect_helper(const endpoints& peers, client* cl)
		{
			std::set<network::address_v4> peer_addrs;
            
            for (auto it = peers.begin();it!= peers.end(); ++it )
            {
                if(!(*it).second)
                {
                    cl->cons_[(*it).first].reset(new async_connector((*it).first, boost::bind(&client::on_connected, cl, _1, _2), [=](error_code const& err){}, [=](error_code const& err){}));
                    peer_addrs.insert((*it).first.addr);
                }
                else
                {
                    cl->cons_[(*it).first].reset(new async_connector((*it).first, boost::bind(&client::on_connected, cl, _1, _2), tcp_error, tcp_error));
                }

                LogInfo("Connecting to " << (*it).first);
			}
            
            for (auto it = peer_addrs.begin();it!= peer_addrs.end(); ++it )
            {
               cl->eps_.push_back(endpoint(*it,0));
            }
             
		}
	};

    struct net_configurer
    {
       struct host_props
       {
           net_layer::host_t                    host;
           net_layer::application_t              app;
           net_layer::configuration_t::task     task;
       };

       typedef  std::vector< host_props >  hosts_props_t;

       net_configurer(endpoints& peers)
           : cfgr_    (net_layer::create_configurator(123)) 
       {
           cfgr_->load_config("3vis.ncfg", cfg_);
           refill_peers(peers);
       }
       
       net_layer::hosts_t const& hosts() const
       {
             return  cfgr_->hosts();
       }
       
       net_layer::configuration_t const&  configuration() const
       {
           return  cfg_;
       }

       net_layer::applications_t  const&  applications () const
       {
            return cfgr_->applications();
       }

       hosts_props_t& get_visas()
       {
           net_layer::configuration_t const& cfg =  configuration();

           for (auto it = cfg.tasks.begin();it!= cfg.tasks.end();++it)
           {
               host_props hp; 
               auto app  = applications ()[(*it).app_id];

               if(app.name == "visapp")
               {
                    auto phosts = hosts();
                    hp.host = phosts[(*it).host_id];
                    hp.app  = app;
                    hp.task = *it;
                    hp_.emplace_back(std::move(hp));
               }

           }

           return hp_;
       }

    private:

        void refill_peers(endpoints& peers)
        {
            net_layer::configuration_t const& cfg =  configuration();
            
            endpoints peers_out;

            for (auto it = cfg.tasks.begin();it!= cfg.tasks.end();++it)
            {
                auto app  = applications ()[(*it).app_id];
                
                if(app.name == "visapp")
                {
                    auto phosts = hosts();
                    auto host = phosts[(*it).host_id];
                    endpoint ep (host.ip + ":45003");
                    peers_out.emplace_back(make_pair(std::move(ep),false));    
                } else if(app.name == "modapp")
                {
                    auto phosts = hosts();
                    auto host = phosts[(*it).host_id];
                    endpoint ep (host.ip + ":45001");
                    peers_out.emplace_back(make_pair(std::move(ep),true));    
                }                
            }

            if(peers_out.size()>0)
               peers = std::move(peers_out);
        }

    private:
        net_layer::configurator_ptr                                            cfgr_;
        net_layer::configuration_t                                              cfg_;
        hosts_props_t                                                            hp_;
    };

    client(endpoints peers)
        : net_cfgr_ (boost::make_shared<net_configurer>(peers))  
        , connect_helper_ (peers, this)
        , period_   (.5)
        , timer_    (boost::bind(&client::update, this))
        , traj_     (fill_trajectory(krv::data_getter("log_minsk.txt")))
        , traj2_    (fill_trajectory(krv::data_getter("log_e_ka50.txt")))
        , traj_pos_ (fill_trajectory(krv::data_getter("log_e_su_posadka.txt")))
        , traj_trp_ (fill_trajectory(krv::data_getter("log_e_su_vzlet_tramplin5.txt")))
        , traj_cam_ (fill_trajectory())
    {
        disp_
            .add<ready_msg                 >(boost::bind(&client::on_remote_ready      , this, _1))
            ;

        const double time = 0.0;

         
#if 1
        ADD_EVENT(0.0  , create(1500, point_3f(-2383.51,-524.20, 21 ), quaternion(cprf(-5.10,0,0)) , ok_camera, "camera 0", "") )
        ADD_EVENT(0.0  , create(1501, point_3f(48.89,411.65,28.50), quaternion(cprf(130, 0, 0)) , ok_camera, "camera 1", "") )
#endif

        ADD_EVENT(time , state(0.0,time,factor))

#if 1
        ADD_EVENT(10.0 , create(333, cg::point_3(0.0,0.0,150.0),traj_->curs_value(traj_->base_length()),ok_flock_of_birds, "crow", "", 50)) 
#if 0
        ADD_EVENT(25.0 , destroy_msg(333)) 
#endif
#endif
        

        environment::weather_t  weather; 
        weather.fog_density  = 0.2f; 
        weather.clouds_type  = static_cast<unsigned>(av::weather_params::cirrus);
        weather.wind_dir     = cg::point_2f(2.0, 4.0);

        ADD_EVENT(2.0, environment_msg(weather))

#if 1

#if 1
        weather.fog_density  = 0.4f; 
        weather.wind_speed  = 20.0f;
        weather.wind_dir    = cg::point_2f(1.0,0.0);
        weather.clouds_type  = static_cast<unsigned>(av::weather_params::cirrus);
        ADD_EVENT(18.0, environment_msg(weather))

        weather.clouds_type  = static_cast<unsigned>(av::weather_params::overcast);
        ADD_EVENT(20.0, environment_msg(weather))

        weather.wind_speed  = 20.0f;
        weather.wind_dir    = cg::point_2f(-1.0,0.0);
        weather.rain_density  = 0.2f;
        ADD_EVENT(25.0, environment_msg(weather))
        weather.rain_density  = 0.5f;
        weather.lightning_intensity  = 0.75f;
        ADD_EVENT(30.0, environment_msg(weather))
        weather.rain_density  = 0.75f;
        ADD_EVENT(35.0, environment_msg(weather))
        weather.rain_density  = 1.0f;
        ADD_EVENT(45.0, environment_msg(weather))

        weather.wind_speed  = 10.0f;
        weather.wind_dir    = cg::point_2f(1.0,1.0);
        weather.rain_density  = 0.75f;
        ADD_EVENT(50.0, environment_msg(weather))
        weather.rain_density  = 0.45f;
        ADD_EVENT(55.0, environment_msg(weather))
        weather.lightning_intensity  = 0.0f;
        weather.rain_density  = 0.2f;
        ADD_EVENT(57.0, environment_msg(weather))
        
        weather.fog_density  = 0.45f; 
        weather.rain_density  = 0.1f;
        ADD_EVENT(60.0, environment_msg(weather))
        
        weather.fog_density  = 0.40f;
        ADD_EVENT(70.0, environment_msg(weather)) 
        weather.rain_density  = 0.0f;
        weather.fog_density  = 0.2f;
        weather.wind_speed  = 0.0f;
        weather.wind_dir    = cg::point_2f(1.0,1.0);
        weather.clouds_type  = static_cast<unsigned>(av::weather_params::none);
        ADD_EVENT(80.0, environment_msg(weather))

        weather.fog_density  = 0.2f;
        weather.wind_speed  = 0.0f;
        weather.wind_dir    = cg::point_2f(1.0,1.0);
        weather.clouds_type  = static_cast<unsigned>(av::weather_params::overcast);
        ADD_EVENT(90.0, environment_msg(weather))
#endif


#if 1
        ADD_EVENT(1.0  , create(1,traj_->kp_value(traj_->base_length()),traj_->curs_value(traj_->base_length()), ok_aircraft, "A319", "1") )
#endif       

#if 1
        ADD_EVENT(10.0 , create(2,traj_->kp_value(traj_->base_length()) + cg::point_3(10.0,10.0,0.0),traj_->curs_value(traj_->base_length()),ok_vehicle,"pojarka", "2")) // "niva_chevrolet"
        ADD_EVENT(10.0,  malfunction_msg(1,MF_FIRE_ON_BOARD,true)) 
        ADD_EVENT(70.0, fire_fight_msg_t(2))
#if 1
        ADD_EVENT(10.0 , create(3,traj_->kp_value(traj_->base_length())+ cg::point_3(10.0,10.0,150.0),traj_->curs_value(traj_->base_length()),ok_flock_of_birds,"crow","", 70)) 
        ADD_EVENT(50.0 , create(4, cg::point_3(0.0,0.0,0.0),traj_->curs_value(traj_->base_length()),ok_flock_of_birds,"crow", "", 20)) 

		ADD_EVENT(52.0 , destroy_msg(3)) 
        ADD_EVENT(80.0 , destroy_msg(4)) 
#endif

#endif

#if 0
        ADD_EVENT(3.0  , create(10,point_3(0,250,0),cg::cpr(0), ok_aircraft, "A319") )
        ADD_EVENT(4.0  , create(11,cg::point_3(0 + 15,250 + 15,0),cg::cpr(0), ok_vehicle, "buksir") )
        ADD_EVENT(30.0 , attach_tow_msg_t(11) )
#endif

#if 0
        ADD_EVENT(10.0  , create(31,traj_->kp_value(traj_->base_length()),traj_->curs_value(traj_->base_length()), ok_human, "human") )
#endif

#if 0
        ADD_EVENT(89.0 , state(0.0,89.,0.0))
#endif


#endif

#if 1

#if 0
        ADD_EVENT(10.0  , create(151,point_3(-435,162,0),cg::cpr(353), ok_helicopter, "KA50", "151") )
        ADD_EVENT(11.0  , create(152,point_3(-485,309,0),cg::cpr(353), ok_helicopter, "KA50", "152") )

        ADD_EVENT(12.0  , create(153,point_3(-466,158,0),cg::cpr(353), ok_helicopter, "KA52", "153") )
        ADD_EVENT(13.0  , create(154,point_3(-478,254,0),cg::cpr(173), ok_helicopter, "KA52", "154") )
#endif

#if 0
        ADD_EVENT(14.0  , create(155,point_3(-415,262,0),cg::cpr(0)  , ok_helicopter, "KA50", "155") )
        ADD_EVENT(15.0  , create(156,point_3(-497,407,0),cg::cpr(0)  , ok_helicopter, "KA50", "156") )
        ADD_EVENT(16.0  , create(157,point_3(-422,318,0),cg::cpr(0)  , ok_helicopter, "KA50", "157") )
        ADD_EVENT(17.0  , create(158,point_3(-357,431,0),cg::cpr(0)  , ok_helicopter, "KA50", "158") )
        ADD_EVENT(18.0  , create(159,point_3(-333,451,0),cg::cpr(0)  , ok_helicopter, "KA50", "159") )
        ADD_EVENT(19.0  , create(160,point_3(-307,470,0),cg::cpr(0)  , ok_helicopter, "KA50", "160") )
#endif
        
        for (int i=0;i<4;i++)
        {
            ADD_EVENT(20.0 + i  , engine_state_msg(151 + i , ES_LOW_THROTTLE)  )
            ADD_EVENT(40.0 + i  , engine_state_msg(151 + i , ES_FULL_THROTTLE) )
            ADD_EVENT(60.0 + i  , engine_state_msg(151 + i , ES_STOPPED) )
        }

#if 1
        ADD_EVENT(12.0  , create(171,point_3(156,387,0),cg::cpr(173), ok_aircraft, "L39", "171") )
        //ADD_EVENT(13.0  , create(172,point_3(322,404,0),cg::cpr(173), ok_aircraft, "L39", "172") )
        //ADD_EVENT(14.0  , create(173,point_3(587,437,0),cg::cpr(173), ok_aircraft, "L39", "173") )   ]
        ADD_EVENT(13.0  , create(173,traj_pos_->kp_value(traj_pos_->base_length()),traj_pos_->curs_value(traj_pos_->base_length()), ok_aircraft, "L39", "173") )
        ADD_EVENT(14.0  , create(172,traj_trp_->kp_value(traj_trp_->base_length()),traj_trp_->curs_value(traj_trp_->base_length()), ok_aircraft, "L39", "172") )
        
#endif

#if 1
		ADD_EVENT(12.0  , create(176,point_3(201,392,0),cg::cpr(173), ok_aircraft, "AN140", "176") )
		ADD_EVENT(13.0  , create(177,point_3(245,398,0),cg::cpr(173), ok_aircraft, "AN140", "177") )
		ADD_EVENT(14.0  , create(178,point_3(286,400,0),cg::cpr(173), ok_aircraft, "AN140", "178") )

		for (int i=0;i<3;i++)
		{
			ADD_EVENT(20.0 + i  , engine_state_msg(176 + i , ES_LOW_THROTTLE)  )
			ADD_EVENT(40.0 + i  , engine_state_msg(176 + i , ES_FULL_THROTTLE) )
			ADD_EVENT(60.0 + i  , engine_state_msg(176 + i , ES_STOPPED) )
		}
#endif


#endif


#if 1
		ADD_EVENT(1.0  , create(150,point_3(-447,258,0),cg::cpr(173), ok_helicopter, "KA27", "150") )

		ADD_EVENT(traj2_->base_length()         , engine_state_msg(150 , ES_LOW_THROTTLE)  )
		ADD_EVENT(traj2_->base_length() + 40.0  , engine_state_msg(150 , ES_FULL_THROTTLE) )
		ADD_EVENT(traj2_->length()              , engine_state_msg(150 , ES_STOPPED) )

		//ADD_EVENT(60.0 + 10.0  , engine_state_msg(150 , ES_LOW_THROTTLE)  )
		//ADD_EVENT(60.0 + 30.0  , engine_state_msg(150 , ES_FULL_THROTTLE) )
		//ADD_EVENT(60.0 + 50.0  , engine_state_msg(150 , ES_STOPPED) )
#endif

        run_f_ = [this](uint32_t id, double time, double traj_offset)->void {
            binary::bytes_t msg =  std::move(network::wrap_msg(run(
                id 
                , traj_->kp_value    (time)
                , traj_->curs_value  (time)
                , *traj_->speed_value(time)
                , time + traj_offset
                , false
                , meteo::local_params()
                )));

            this->send(&msg[0], msg.size());
        };

        run_f2_ = [this](uint32_t id, double time, double traj_offset)->void {
            binary::bytes_t msg =  std::move(network::wrap_msg(run(
                id 
                , traj2_->kp_value    (time)
                , traj2_->curs_value  (time)
                , *traj2_->speed_value(time)
                , time + traj_offset
                , false
                , meteo::local_params()
                )));

            this->send(&msg[0], msg.size());
        };

        run_f_trp_ = [this](uint32_t id, double time, double traj_offset)->void {
            binary::bytes_t msg =  std::move(network::wrap_msg(run(
                id 
                , traj_trp_->kp_value    (time)
                , traj_trp_->curs_value  (time)
                , *traj_trp_->speed_value(time)
                , time + traj_offset
                , false
                , meteo::local_params()
                )));

            this->send(&msg[0], msg.size());
        };

        run_f_pos_ = [this](uint32_t id, double time, double traj_offset)->void {
            binary::bytes_t msg =  std::move(network::wrap_msg(run(
                id 
                , traj_pos_->kp_value    (time)
                , traj_pos_->curs_value  (time)
                , *traj_pos_->speed_value(time)
                , time + traj_offset
                , false
                , meteo::local_params()
                )));

            this->send(&msg[0], msg.size());
        };
        
        runs_once_.insert(make_pair( 1,
            [this]( double time )->void {

                client::net_configurer::hosts_props_t& hp = net_cfgr_->get_visas();

                for (auto it = hp.begin(); it!=hp.end(); ++it )
                {
                    for (auto it_p = peers_.begin(); it_p!=peers_.end();++it_p)
                        if((*it).host.ip==it_p->first.addr.to_string())
                        {
                            binary::bytes_t bts =  std::move(network::wrap_msg(props_updated((*it).task.properties)));
                            it_p->second->send(&bts[0], bts.size());
                        }
                }	
        }
        ));

        runs_once_.insert(make_pair( 25,
            [this]( double time )->void {

            client::net_configurer::hosts_props_t& hp = net_cfgr_->get_visas();

            for (auto it = hp.begin(); it!=hp.end(); ++it )
            {
                for (auto it_p = peers_.begin(); it_p!=peers_.end();++it_p)
                if((*it).host.ip==it_p->first.addr.to_string())
                {
                    kernel::vis_sys_props props;
                    std::stringstream is((*it).task.properties);
                    prop_tree::read_from(is, props);
                    props.channel.camera_name = "camera 1";
                    std::stringstream os;
                    prop_tree::write_to(os, props); 

                    binary::bytes_t bts =  std::move(network::wrap_msg(props_updated(os.str())));
                    it_p->second->send(&bts[0], bts.size());
                }
            }	
        }
        ));
        
        runs_.insert(make_pair(traj_cam_->base_length(),
                    [this](double time)->void {
                        uint32_t id =  1501;
                        double traj_offset = 0.0;
                        binary::bytes_t msg =  std::move(network::wrap_msg(run(
                            id 
                            , traj_cam_->kp_value    (time)
                            , traj_cam_->curs_value  (time)
                            , *traj_cam_->speed_value(time)
                            , time + traj_offset
                            , false
                            , meteo::local_params()
                            )));

                        this->send(&msg[0], msg.size());
                }
            ));

        runs_.insert(make_pair(traj_trp_->base_length(),
            boost::bind( run_f_trp_ , /*172*/178, _1, /*traj_offset*/0)
            ));

        runs_.insert(make_pair(traj_pos_->base_length(),
            boost::bind( run_f_pos_, 173,_1,traj_offset)
            ));


        runs_.insert(make_pair(traj_->base_length(),
            boost::bind( run_f_ , 1,_1,traj_offset)
            ));

        runs_.insert(make_pair(traj2_->base_length(),
            boost::bind( run_f2_, 150,_1,traj_offset)
            ));

        runs_.insert(make_pair( traj_->base_length() - vehicle_prediction,
            [this] (double time)
            {
                double vtime = time + vehicle_prediction;

                //run_f_(2,vtime,0);
                //run_f_(3,vtime,0);

                run_f_(31,vtime,0);

               //LogInfo("update() send run " << _traj->base_length() << "  " <<time << "  x: " << _traj->kp_value(vtime).x 
                //                                     << "  y: " << _traj->kp_value(vtime).y 
                //                                     << "  h: " << _traj->kp_value(vtime).z
                //                                     <<  "\n");
           } 
         ));

#if 0         
         cg::point_3 poss[] = {cg::point_3(0.907 * 1000 , -0.060 * 1000, 0.0), 
                               cg::point_3(0.883 * 1000 , -0.030 * 1000, 0.0),
                               cg::point_3(0.9345 * 1000, -0.0915 * 1000, 0.0),
                               cg::point_3(0.982 * 1000 , -0.152 * 1000, 0.0),
                               cg::point_3(0.962 * 1000 , -0.124 * 1000, 0.0)
         };


        const cg::quaternion    orien (cg::cprf(49.0) );
        for (int i = 0; i <5; i++)
        {
            ADD_EVENT(15.0 + i  , create(10 + i,poss[i],orien, ok_aircraft, "SU25") ) // su_25tm
        }
#endif


    }

    // from struct tcp_connection
    void send(void const* data, uint32_t size)
    {
        error_code_t ec;
		for (auto it = peers_.begin();it!= peers_.end(); ++it )
		{
			(*it).second->send(data, size);
			if (ec)
			{
				LogError("TCP send error: " << ec.message());
				//return;
			}
		}
    }

private:
    void on_connected(network::tcp::socket& sock, network::endpoint const& peer)
    {
        LogInfo("Connected to " << peer);
        

        peers_[peer] = std::shared_ptr<tcp_fragment_wrapper>(new tcp_fragment_wrapper(
            sock, boost::bind(&msg_dispatcher<endpoint>::dispatch, &disp_, _1, _2, peer), &tcp_error, &tcp_error));  
        
        if (peer.port == 45001)
        {
            binary::bytes_t bts =  std::move(wrap_msg(vis_peers(eps_)));
            peers_[peer]->send(&bts[0], bts.size());
            LogInfo("Send peers list to " << peer);
        }
        

        {
            net_configurer::hosts_props_t& hp = net_cfgr_->get_visas();

            for (auto it = hp.begin(); it!=hp.end(); ++it )
            {
                if((*it).host.ip==peer.addr.to_string())
                {
                    kernel::vis_sys_props props;
                    // props.base_point = *kta_position( "URSS" );
                    // props.channel.course = 60 ;
                    // std::stringstream os;
                    // prop_tree::write_to(os, props); 

                    binary::bytes_t bts =  std::move(wrap_msg(props_updated(/*os.str()*/(*it).task.properties)));
                    peers_[peer]->send(&bts[0], bts.size());
                }
            }		   

        }

        {
            binary::bytes_t bts =  std::move(wrap_msg(setup(g_icao_code)));
            peers_[peer]->send(&bts[0], bts.size());
        }

    }

    void on_remote_ready(uint16_t value)
    {
        start_send();
    }
                                  
    inline void start_send()
    {
        update();
    }

    void update()
    {   
        static double time = 0;
       
       
        for(time_queue_msgs_t::iterator it = msgs_.begin(); it != msgs_.end(); ) {
            if (it->first <= time) {
                send(&it->second[0], it->second.size());
                msgs_.erase(it++);
            } else {
                ++it;
            }
        }

        typedef std::multimap<double, run_f>  time_queue_run_t;
        time_queue_run_t                         run_queque_f_;

        for(time_queue_run_t::iterator it = runs_.begin(); it != runs_.end(); ++it ) {
            if (it->first <= time) {
                it->second(time);
            }
        }

        std::vector<time_queue_run_t::iterator> to_delete;
        for(time_queue_run_t::iterator it = runs_once_.begin(); it != runs_once_.end(); ++it ) {
            if (it->first <= time) {
                it->second(time);
                to_delete.push_back(it);
            }
        }
        
        for(auto it = to_delete.begin(); it != to_delete.end(); ++it ) {
             runs_once_.erase(*it);
        }

#if 0
        if(time>=_traj->base_length())
        {
            run_f_(1,time,traj_offset);
           
#if 0
            if (messages_size_ + binary::size(msg) > /*msg_threshold_*/100)
            {
                binary::bytes_t bts =  network::wrap_msg(net_layer::msg::container_msg(move(messages_)));

                send(&bts[0], bts.size()); 

                messages_.clear();
                messages_size_ = 0;
            }

            
            messages_size_ += binary::size(msg);
            messages_.push_back(move(msg));
#endif
        }
#endif
        
        time += period_ * factor;

        timer_.wait(boost::posix_time::microseconds(int64_t(1e6 * period_)));
    }


private: 
    boost::shared_ptr<net_configurer>                                net_cfgr_;

private:
    net_layer::msg::endpoints                                            eps_;

private:
    async_timer             timer_; 
    double                  period_;

private:
    std::map< endpoint,unique_ptr<async_connector>>                      cons_;
	std::map< endpoint, std::shared_ptr<tcp_fragment_wrapper> >         peers_;
	connect_helper                                             connect_helper_;
    
	msg_dispatcher<network::endpoint>                                    disp_;
    net_layer::msg::container_msg::msgs_t                            messages_;
    size_t                                                      messages_size_;

private:
    
    fms::trajectory_ptr                                                  traj_;
    fms::trajectory_ptr                                                 traj2_;
    fms::trajectory_ptr                                              traj_pos_;
    fms::trajectory_ptr                                              traj_trp_;
    fms::trajectory_ptr                                              traj_cam_;

    typedef std::multimap<double, bytes_t>  time_queue_msgs_t;
    time_queue_msgs_t                                                    msgs_;
    
    typedef boost::function<void(uint32_t,double,double)>           run_wrap_f;

    run_wrap_f                                                          run_f_;
    run_wrap_f                                                         run_f2_;
    run_wrap_f                                                      run_f_trp_;
    run_wrap_f                                                      run_f_pos_;
                                                          

    typedef boost::function<void(double /*time*/)>   run_f;

    typedef std::multimap<double, run_f>  time_queue_run_t;
    time_queue_run_t                                                     runs_;
    time_queue_run_t                                                runs_once_;

};


int _tmain(int argc, _TCHAR* argv[])
{
    async_services_initializer asi(false);
    
    logging::add_console_writer();

    __main_srvc__ = &(asi.get_service());
    
    cmd_line::arg_map am;
    if (!am.parse(cmd_line::naive_parser().add_arg("icao_code", true), argc, argv))
    {
        LogError("Invalid command line");
        return 1;
    }
    
    g_icao_code = am.extract<string>("icao_code"    , "URSS");

    try
    {
        client::endpoints ep;
#if 0
		ep.emplace_back(make_pair(endpoint(std::string("127.0.0.1:45001")), true));            // ModApp
#if 0
//        ep.emplace_back(make_pair(endpoint(std::string("127.0.0.1:45003")), false));         // VisApp
        ep.emplace_back(make_pair(endpoint(std::string("192.9.206.141:45003")), false));       // VisApp
        ep.emplace_back(make_pair(endpoint(std::string("192.9.206.142:45003")), false));       // VisApp
        ep.emplace_back(make_pair(endpoint(std::string("192.9.206.143:45003")), false));       // VisApp

#else
        ep.emplace_back(make_pair(endpoint(std::string("127.0.0.1:45003")), false));           // VisApp
        //ep.emplace_back(make_pair(endpoint(std::string("192.9.206.245:45003")), false));     // VisApp
#endif
#endif

        client c(ep);              
        __main_srvc__->run();
    }
    catch(verify_error const&)
    {
        return -1;
    }
    catch(std::exception const& err)
    {
        LogError("Exception caught: " << err.what());
        return -1;
    }

	return 0;
}

