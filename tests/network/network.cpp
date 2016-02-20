// network.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "ipc/ipc.h"

#include "common/cmd_line.h"

#include "async_services/async_services.h"
#include "network/msg_dispatcher.h"
#include "logger/logger.hpp"
#include "common/test_msgs.h"

#include "utils/krv_import.h"

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

    const unsigned start_idx = 0/*11*/;
    cg::point_2 prev(kdg.kd_[start_idx].x,kdg.kd_[start_idx].y);
    double tlength = 0;
    for(auto it = kdg.kd_.begin() + start_idx; it!= kdg.kd_.end();++it )
    {
        auto p = cg::point_2(it->x,it->y);
        auto dist = cg::distance(prev,p);
        tlength += dist;
        crs.insert (std::make_pair(/*tlength*/it->time,cpr(it->fiw,it->tg, it->kr )));
        kpts.insert(std::make_pair(/*tlength*/it->time,cg::point_3(p,it->h)));

        if(it != kdg.kd_.begin())
            vls.insert(std::make_pair(/*tlength*/it->time,dist/(it->time - std::prev(it)->time)));

        prev = p;
    }

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


struct client
{

    DECL_LOGGER("visa_control");
	
	bool create_a;
	bool create_v;
	bool create_f;

    typedef std::vector<endpoint> endpoints;
	
	struct connect_helper
	{
		connect_helper(endpoints peers, client* cl)
		{
			for (auto it = peers.begin();it!= peers.end(); ++it )
            {
				cl->cons_[*it].reset(new async_connector(*it, boost::bind(&client::on_connected, cl, _1, _2), tcp_error, tcp_error));
                LogInfo("Connecting to " << *it);
			}

		}
	};

    client(endpoints peers)
        : connect_helper_ (peers, this)
        , period_  (/*4.*/.5)
        , timer_   (boost::bind(&client::update, this))
        , _traj    (fill_trajectory(krv::data_getter("log_minsk.txt")))
		, create_a (true)
	    , create_v (true)
		, create_f (true)
    {
        disp_
            .add<ready_msg                 >(boost::bind(&client::on_remote_ready      , this, _1))
            ;
        
        const double time = 0.0;

        ADD_EVENT(time , state(0.0,time,factor))
#if 1
        ADD_EVENT(1.0  , create(1,_traj->kp_value(_traj->base_length()),_traj->curs_value(_traj->base_length()), ok_aircraft, "A319") )
        ADD_EVENT(70.0, fire_fight_msg_t(2))
#endif       

#if 1
        ADD_EVENT(10.0 , create(2,_traj->kp_value(_traj->base_length()) + cg::point_3(10.0,10.0,0.0),_traj->curs_value(_traj->base_length()),ok_vehicle,"pojarka")) // "niva_chevrolet"
        ADD_EVENT(10.0 , create(3,_traj->kp_value(_traj->base_length()),_traj->curs_value(_traj->base_length()),ok_flock_of_birds,"crow")) 
        ADD_EVENT(10.0,  malfunction_msg(1,MF_FIRE_ON_BOARD,true)) 
#endif

#if 0
        ADD_EVENT(3.0  , create(10,point_3(0,250,0),cg::cpr(0), ok_aircraft, "A319") )
        ADD_EVENT(4.0  , create(11,cg::point_3(0 + 15,250 + 15,0),cg::cpr(0), ok_vehicle, "buksir") )
        ADD_EVENT(30.0 , attach_tow_msg_t(11) )
#endif

        ADD_EVENT(10.0  , create(31,_traj->kp_value(_traj->base_length()),_traj->curs_value(_traj->base_length()), ok_human, "human") )

#if 0
        ADD_EVENT(25.0 , state(0.0,25.,0.0))
#endif
        ADD_EVENT(1.0  , create(150,point_3(0,250,0),cg::cpr(0), ok_helicopter, "KA27") )
        
        ADD_EVENT(20.0  , engine_state_msg(150 , ES_LOW_THROTTLE)  )
        ADD_EVENT(40.0  , engine_state_msg(150 , ES_FULL_THROTTLE) )
        ADD_EVENT(60.0  , engine_state_msg(150 , ES_STOPPED) )

        ADD_EVENT(60.0 + 10.0  , engine_state_msg(150 , ES_LOW_THROTTLE)  )
        ADD_EVENT(60.0 + 30.0  , engine_state_msg(150 , ES_FULL_THROTTLE) )
        ADD_EVENT(60.0 + 50.0  , engine_state_msg(150 , ES_STOPPED) )

        run_f_ = [this](uint32_t id, double time, double traj_offset)->void {
            binary::bytes_t msg =  std::move(network::wrap_msg(run(
                id 
                ,_traj->kp_value    (time)
                ,_traj->curs_value  (time)
                ,*_traj->speed_value(time)
                , time + traj_offset
                , false
                , meteo::local_params()
                )));

            this->send(&msg[0], msg.size());
        };


        runs_.insert(make_pair(_traj->base_length(),                                 
            boost::bind( run_f_, 1,_1,traj_offset)
            ));

        runs_.insert(make_pair( _traj->base_length() - vehicle_prediction,
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
      
        binary::bytes_t bts =  std::move(wrap_msg(setup(g_icao_code)));
        send(&bts[0], bts.size());

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
    
    fms::trajectory_ptr                              _traj;
    typedef std::multimap<double, bytes_t>  time_queue_msgs_t;
    time_queue_msgs_t                                msgs_;
    boost::function<void(uint32_t,double,double)>   run_f_;

    typedef boost::function<void(double /*time*/)>    run_f;

    typedef std::multimap<double, run_f>  time_queue_run_t;
    time_queue_run_t                                  runs_;



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
		ep.emplace_back(endpoint(std::string("127.0.0.1:45001")));
        ep.emplace_back(endpoint(std::string("127.0.0.1:45003")));
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

