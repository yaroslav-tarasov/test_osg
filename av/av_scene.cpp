#include "stdafx.h"

#include "async_services/async_services.h"
#include "network/msg_dispatcher.h"
#include "kernel/systems.h"
#include "kernel/systems/systems_base.h"
#include "kernel/systems/fake_system.h"
#include "kernel/object_class.h"
#include "kernel/msg_proxy.h"
#include "common/test_msgs.h"

#include "kernel/systems/vis_system.h"

#include "av/Visual.h"
#include "av/test_systems.h"

using network::endpoint;
using network::async_acceptor;
using network::async_connector;
using network::tcp_socket;
using network::error_code;

using network::msg_dispatcher;
using network::tcp_fragment_wrapper;

using namespace net_layer::test_msg;

namespace
{
    boost::asio::io_service* __main_srvc__ = 0;

    void tcp_error(error_code const& err)
    {
        LogError("Error happened: " << err);
        __main_srvc__->stop();
    }

    struct updater
    {
       updater(kernel::system_ptr  sys,double period = 0.01)
          : period_(period)
          , old_val(0)
          , sys_(sys)
       {

       }

       __forceinline void update(double time)
       {
           if(time - old_val >=period_)
           {
               sys_->update(time);
               old_val = time;
           }
       }

       __forceinline void reset()
       {
           sys_.reset();
       }


    private:
       double                                                   period_; 
       double                                                   old_val;
       kernel::system_ptr                                          sys_;
    };

}

struct visapp
{
    visapp(endpoint peer, kernel::vis_sys_props const& props/*, binary::bytes_cref bytes*/,int argc, char** argv)
        : acc_      (peer, boost::bind(&visapp::on_accepted, this, _1, _2), tcp_error)
        , period_   (0.01)
        , osg_vis_  (CreateVisual())
        , vis_sys_  (create_vis(props/*, bytes*/))
        , ctrl_sys_ (sys_creator()->get_control_sys(),cfg().model_params.csys_step)
        , mod_sys_  (sys_creator()->get_model_sys(),cfg().model_params.msys_step)
        , timer_    (boost::bind(&visapp::update, this))
        
    {   

        disp_
            .add<setup                 >(boost::bind(&visapp::on_setup      , this, _1))
            .add<run                   >(boost::bind(&visapp::on_run        , this, _1))
            .add<create                >(boost::bind(&visapp::on_create     , this, _1))
            ;
        
        FIXME( Arguments );
        
        osg_vis_->Initialize(argc,argv);
        
        FIXME(Splash screen and scene with properties) 
//      Сначала сцена потом автообъекты

        create_objects();


        update();
    }

    ~visapp()
    {
       vis_sys_.reset();
    }

private:
    void update()
    {   
        double sim_time = osg_vis_->GetInternalTime();
        if(sim_time  < 0)
        {
           timer_.cancel();
           __main_srvc__->stop();
           return;
        }
        else
           timer_.wait(boost::posix_time::microseconds(int64_t(1e6 * period_)));

        
        mod_sys_.update(sim_time);
        ctrl_sys_.update(sim_time);
        vis_sys_.update(sim_time);
        osg_vis_->Render();

    }

    void on_accepted(network::tcp::socket& sock, endpoint const& peer)
    {
        //client_ = in_place(boost::ref(sock), network::on_receive_f(), boost::bind(&server::on_disconnected,this,_1)/*&tcp_error*/, &tcp_error);
        
        uint32_t id = next_id();
        sockets_[id] = std::shared_ptr<tcp_fragment_wrapper>(new tcp_fragment_wrapper(
            sock, boost::bind(&msg_dispatcher<uint32_t>::dispatch, &disp_, _1, _2, id),
            boost::bind(&visapp::on_disconnected, this, _1, id),
            boost::bind(&visapp::on_error, this, _1, id)));        
        
        LogInfo("Client " << peer << " accepted");
    }

    void on_disconnected(error_code const& ec)      
    {
         LogInfo("Client " << " disconnected with error: " << ec.message() );
    }
    
    void on_disconnected(boost::system::error_code const& ec, uint32_t sock_id)
    {  
        LogInfo("Client " << sock_id << " disconnected with error: " << ec.message() );
        sockets_.erase(sock_id);
        // peers_.erase(std::find_if(peers_.begin(), peers_.end(), [sock_id](std::pair<id_type, uint32_t> p) { return p.second == sock_id; }));
    }
    
    void on_error(boost::system::error_code const& ec, uint32_t sock_id)
    {
        LogError("TCP error: " << ec.message());
        sockets_.erase(sock_id);
        // peers_.erase(std::find_if(peers_.begin(), peers_.end(), [sock_id](std::pair<id_type, uint32_t> p) { return p.second == sock_id; }));
    }
    
    void on_setup(setup const& msg)
    {
         LogInfo("Got setup message: " << msg.srv_time << " : " << msg.task_time );
    }

    void on_run(run const& msg)
    {
        LogInfo("Got run message: " << msg.srv_time << " : " << msg.task_time );
    }

    void on_create(create const& msg)
    {
        auto fp = fn_reg::function<void(create const&)>("create_aircraft");
        if(fp)
            fp(msg);

        LogInfo("Got create message: " << msg.course << " : " << msg.lat << " : " << msg.lon  );
    }


    uint32_t next_id()
    {
        static uint32_t id = 0;
        return id++;
    }

    void create_objects()
    {
        using namespace binary;
        using namespace kernel;

        sys_creator()->create_auto_objects();

        auto fp = fn_reg::function<void(void)>("create_objects");
        if(fp)
            fp();
    }

    kernel::visual_system_ptr create_vis(kernel::vis_sys_props const& props/*, binary::bytes_cref bytes*/)
    {
        using namespace binary;
        using namespace kernel;

        //systems_factory_ptr sys_fab = nfi::function<systems_factory_ptr()>("systems", "create_system_factory")();
        auto ptr = // /*sys_fab->*/create_visual_system(msg_service_, /*vw_->get_victory(),*/ props);
                   sys_creator()->get_visual_sys();
        //dict_t dic;
        //binary::unwrap(bytes, dic);

        //system_ptr(ptr)->load_exercise(dic);
        return ptr;
    }



private:
    async_acceptor                                              acc_   ;
    optional<tcp_socket>                                        client_;
    msg_dispatcher<uint32_t>                                    disp_;
    std::map<uint32_t, std::shared_ptr<tcp_fragment_wrapper> >  sockets_;
    //kernel::msg_service                                         msg_service_;

private:
    updater                                                     mod_sys_;
    updater                                                    ctrl_sys_;
    updater                                                     vis_sys_;
    IVisual*                                                    osg_vis_;

private:
    async_timer                                                 timer_; 
    double                                                      period_;

private:
    randgen<> rng_;
};


int av_scene( int argc, char** argv )
{
    
    logger::need_to_log(/*true*/);

    async_services_initializer asi(false);
    
    logging::add_console_writer();

    __main_srvc__ = &(asi.get_service());

    try
    {

        endpoint peer(cfg().network.local_address);

        kernel::vis_sys_props props_;
        props_.base_point = ::get_base();

        visapp s(peer, props_ , argc, argv);
        
        __main_srvc__->run();

    }
    catch(verify_error const&)
    {
        return -1;
    }
    catch(std::exception const& err)
    {
        std::string errror (err.what()); 
        LogError("Exception caught: " << err.what());
        return -1;
    }

    return 0; 

}

AUTO_REG(av_scene)
