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

typedef boost::system::error_code       error_code_t;

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

struct net_worker
{
     typedef boost::function<void(const void* data, size_t size)>   on_receive_f;
     typedef boost::function<void(double time)>                     on_update_f;     

     net_worker(const  endpoint &  peer, on_receive_f on_recv , on_update_f on_update)
         : period_     (0.01)
         , on_receive_ (on_recv)
         , on_update_  (on_update)
         , _peer       (peer)
         , timer_      (boost::bind(&net_worker::on_timer, this))
         , done_       (false)
     {
          _workerThread = boost::thread(&net_worker::run, this);
     }
     
     ~net_worker()
     {
         
         _workerService->post(boost::bind(&boost::asio::io_service::stop, _workerService));
         _workerThread.join();
     }

     boost::asio::io_service* GetService()
     {
         return _workerService;
     }

     void done()
     {
         done_ = true;
     }

     void send(void const* data, uint32_t size)
     {
         _workerService->post(boost::bind(&net_worker::do_send, this, 0, binary::make_bytes_ptr(data, size)));

     }

private:
     void run()
     {
         async_services_initializer asi(false);
         acc_.reset(new  async_acceptor (_peer, boost::bind(&net_worker::on_accepted, this, _1, _2), tcp_error));
         _workerService = &(asi.get_service());
         boost::asio::io_service::work skwark(asi.get_service());
         
         start_timer();

         boost::system::error_code ec;
         size_t ret = _workerService->run(ec);
         
         timer_.cancel();
         acc_.reset();
         sockets_.clear();
     }

     uint32_t next_id()
     {
         static uint32_t id = 0;
         return id++;
     }

     // from struct tcp_connection
     void do_send(int id, binary::bytes_ptr data)
     {   
         size_t size = binary::size(*data);
         error_code_t ec;
         sockets_[id]->send(&size, sizeof(uint32_t));
         if (ec)
         {
             LogError("TCP send error: " << ec.message());
             return;
         }

         sockets_[id]->send(binary::raw_ptr(*data), size);
         if (ec)
         {
             LogError("TCP send error: " << ec.message());
             return;
         }

     }

     void start_timer()
     {
         on_timer();
     }

private:

     void on_timer()
     {
         double time = 0;

         if(done_)
         {
             timer_.cancel();
             return;
         }
         else
             timer_.wait(boost::posix_time::microseconds(int64_t(1e6 * period_)));

         __main_srvc__->post(boost::bind(on_update_, time ));

     }

     void on_accepted(network::tcp::socket& sock, endpoint const& peer)
     {
         uint32_t id = next_id();
         sockets_[id] = std::shared_ptr<tcp_fragment_wrapper>(new tcp_fragment_wrapper(
             sock, boost::bind(&net_worker::on_recieve, this, _1, _2, id),
             boost::bind(&net_worker::on_disconnected, this, _1, id),
             boost::bind(&net_worker::on_error, this, _1, id)));        

         LogInfo("Client " << peer << " accepted");
     }

     void on_recieve(const void* data, size_t size, uint32_t id)
     {
          __main_srvc__->post(boost::bind(&net_worker::do_receive, this, binary::make_bytes_ptr(data, size)) );
          
          LogInfo("on_recieve ");
     }
     
     void do_receive(binary::bytes_ptr data/*, endpoint const& peer*/) 
     {
         if (on_receive_)
             on_receive_(binary::raw_ptr(*data), binary::size(*data)/*, peer*/);
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

private:
    boost::scoped_ptr<async_acceptor>                           acc_   ;
    std::map<uint32_t, std::shared_ptr<tcp_fragment_wrapper> >  sockets_;
private:
    boost::thread                                               _workerThread;
    boost::asio::io_service*                                    _workerService;
    std::shared_ptr<boost::asio::io_service::work>              _work;
    const  endpoint &                                           _peer;


private:
    on_receive_f                                                on_receive_;
    on_update_f                                                 on_update_;

private:
    // std::map<uint32_t, std::shared_ptr<tcp_fragment_wrapper> >  sockets_;
    async_timer                                                   timer_; 

    bool                                                          done_;

private:
    double                                                       period_;
};


struct visapp
{
    visapp(endpoint peer, kernel::vis_sys_props const& props/*, binary::bytes_cref bytes*/,int argc, char** argv)
        : osg_vis_  (CreateVisual())
        , vis_sys_  (create_vis(props/*, bytes*/))
        , ctrl_sys_ (sys_creator()->get_control_sys(),cfg().model_params.csys_step)
        , mod_sys_  (sys_creator()->get_model_sys(),cfg().model_params.msys_step)
        , w_ ( peer 
             , boost::bind(&msg_dispatcher<uint32_t>::dispatch, &disp_, _1, _2, 0/*, id*/)
             , boost::bind(&visapp::update, this, _1)
             )
        
    {   

        disp_
            .add<setup                 >(boost::bind(&visapp::on_setup      , this, _1))
            .add<run                   >(boost::bind(&visapp::on_run        , this, _1))
            .add<create                >(boost::bind(&visapp::on_create     , this, _1))
            ;
        
        FIXME( Arguments );
        
        osg_vis_->Initialize(argc,argv);

    }

    ~visapp()
    {
       vis_sys_.reset();
    }

private:
    void update(double time)
    {   
        double sim_time = osg_vis_->GetInternalTime();

        if(sim_time  < 0)
        {
            w_.done();
           __main_srvc__->stop();
           return;
        }

        
        mod_sys_.update(sim_time);
        ctrl_sys_.update(sim_time);
        vis_sys_.update(sim_time);
        osg_vis_->Render();

        LogInfo("on_recieve ");
    }

   
    void on_setup(setup const& msg)
    {
         osg_vis_->CreateScene();
         create_objects();
         
         binary::bytes_t bts =  std::move(wrap_msg(ready_msg(0)));
         w_.send(&bts[0], bts.size());

         LogInfo("Got setup message: " << msg.srv_time << " : " << msg.task_time );
    }

    void on_run(run const& msg)
    {
        // w_.GetService()->post(boost::bind(&visapp::async_run, this,  msg));  // И никаких ref

        LogInfo("Got run message: " << msg.srv_time << " : " << msg.task_time );
    }
    
    void async_run(run const& msg)
    {
        LogInfo("async_run got run message: " << msg.srv_time << " : " << msg.task_time );
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
    net_worker                                                        w_;

private:
    msg_dispatcher<uint32_t>                                       disp_;

private:
    updater                                                     mod_sys_;
    updater                                                    ctrl_sys_;
    updater                                                     vis_sys_;
    IVisual*                                                    osg_vis_;
};


int av_scene( int argc, char** argv )
{
    
    logger::need_to_log(/*true*/);
    logging::add_console_writer();

    boost::asio::io_service  service_;
    typedef boost::shared_ptr<boost::asio::io_service::work> work_ptr;
    work_ptr dummy_work(new boost::asio::io_service::work(service_));

    __main_srvc__ = &service_;

    try
    {

        endpoint peer(cfg().network.local_address);

        kernel::vis_sys_props props_;
        props_.base_point = ::get_base();

        visapp s(peer, props_ , argc, argv);
        
        boost::system::error_code ec;
        __main_srvc__->run(ec);

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
