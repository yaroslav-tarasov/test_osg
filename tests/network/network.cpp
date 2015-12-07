// network.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

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
    fms::trajectory::speed_t  vls;

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


bool create_a = true;
bool create_v = true;

struct client
{

    DECL_LOGGER("visa_control");

    client(endpoint peer)
        : con_(peer, boost::bind(&client::on_connected, this, _1, _2), tcp_error, tcp_error)
        , period_(/*4.*/.5)
        , timer_  (boost::bind(&client::update, this))
        , _traj(fill_trajectory(krv::data_getter(/*"log_minsk.txt"*/"log_minsk_buks22.txt")))
    {
        LogInfo("Connecting to " << peer);

        disp_
            .add<ready_msg                 >(boost::bind(&client::on_remote_ready      , this, _1))
            ;

    }

    // from struct tcp_connection
    void send(void const* data, uint32_t size)
    {
        error_code_t ec;
        srv_->send(&size, sizeof(uint32_t));
        if (ec)
        {
            LogError("TCP send error: " << ec.message());
            return;
        }

        srv_->send(data, size);
        if (ec)
        {
            LogError("TCP send error: " << ec.message());
            return;
        }

    }

private:
    void on_connected(network::tcp::socket& sock, network::endpoint const& peer)
    {
        LogInfo("Connected to " << peer);
        
        srv_ = std::shared_ptr<tcp_fragment_wrapper>(new tcp_fragment_wrapper(
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
        const  double factor = 1.0;
        const  double traj_offset = 20.;
        const  double vehicle_prediction = 0;
        
        if (time>1 && create_a)
        {
            
            binary::bytes_t bts =  std::move(wrap_msg(create(1,_traj->kp_value(_traj->base_length())/*point_3(0,250,0)*/,_traj->curs_value(_traj->base_length()), ok_aircraft, "A319")));
            send(&bts[0], bts.size());
            LogInfo("update() send create " );
            create_a = false;

            binary::bytes_t msg =  std::move(wrap_msg(state(0.0,time,factor)));
            send(&msg[0], msg.size());
        }

        if(time>_traj->base_length())
        {
            binary::bytes_t msg =  std::move(wrap_msg(run(
                                                            1 
                                                           ,_traj->kp_value(time)
                                                           ,_traj->curs_value(time)
                                                           ,*_traj->speed_value(time)
                                                           , time + traj_offset
                                                           , meteo::local_params()
            )));

            send(&msg[0], msg.size());
           
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


        
#if 1
        if(time > 1 )
        {
            if (create_v)
            {
                binary::bytes_t bts =  std::move(wrap_msg(create(2,_traj->kp_value(_traj->base_length()),_traj->curs_value(_traj->base_length()),ok_vehicle,"buksir"))); // "niva_chevrolet"
                send(&bts[0], bts.size());
                create_v = false;
                LogInfo("update() send create " );
            }


        }

        if(time >= _traj->base_length() - vehicle_prediction)
        {
            double vtime = time + vehicle_prediction;
            binary::bytes_t msg =  std::move(wrap_msg(run(
                2 
                ,_traj->kp_value    (vtime)
                ,_traj->curs_value  (vtime)
                ,*_traj->speed_value(vtime)
                , vtime /*+ traj_offset*/
                , meteo::local_params()
                )));

            send(&msg[0], msg.size());

            //LogInfo("update() send run " << _traj->base_length() << "  " <<time << "  x: " << _traj->kp_value(vtime).x 
            //                                     << "  y: " << _traj->kp_value(vtime).y 
            //                                     << "  h: " << _traj->kp_value(vtime).z
            //                                     <<  "\n");
        }

#endif
        
        time += period_ * factor;

        timer_.wait(boost::posix_time::microseconds(int64_t(1e6 * period_)));
    }

private:
    async_timer             timer_; 
    double                  period_;

private:
    async_connector         con_;
    std::shared_ptr<tcp_fragment_wrapper>  srv_;

    msg_dispatcher<network::endpoint>                                    disp_;
    net_layer::msg::container_msg::msgs_t                            messages_;
    size_t                                                      messages_size_;

private:
    fms::trajectory_ptr      _traj;
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
        endpoint peer(std::string("127.0.0.1:45001"));
        client c(peer);              
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

