#include "stdafx.h"

#include "av/Visual.h"

#include "async_services/async_services.h"
#include "network/msg_dispatcher.h"
#include "kernel/systems.h"
#include "kernel/systems/systems_base.h"
#if 0
#include "kernel/systems/fake_system.h"
#endif
#include "kernel/object_class.h"
#include "kernel/msg_proxy.h"
#include "kernel/systems/vis_system.h"
#include "common/ext_msgs.h"
#include "common/time_counter.h"
#include "utils/high_res_timer.h"
#include "tests/systems/factory_systems.h"
#include "objects/registrator.h"

#include "net_layer/net_worker.h"

using network::endpoint;
using network::async_acceptor;
using network::async_connector;
using network::tcp_socket;
using network::error_code;

typedef boost::system::error_code       error_code_t;

using network::msg_dispatcher;
using network::tcp_fragment_wrapper;

using namespace net_layer::msg;

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
       updater(kernel::system_ptr  sys,double period = 0.001)
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

       __forceinline kernel::system_ptr get_sys()
       {
           return sys_;
       }

    private:
       double                                                   period_; 
       double                                                   old_val;
       kernel::system_ptr                                          sys_;
    };

}

namespace
{

struct net_worker
{
     typedef boost::function<void(const void* data, size_t size)>   on_receive_f;
     typedef boost::function<void(double time)>                     on_update_f;     
     

     net_worker(const  endpoint &  peer, on_receive_f on_recv , on_update_f on_update, on_update_f on_render)
         : period_     (cfg().model_params.msys_step)
         , ses_        (net_layer::create_session(binary::bytes_t(), true))
         , mod_peer_   (peer)
         , on_receive_ (on_recv)
         , on_update_  (on_update)
         , on_render_  (on_render)
         , done_       (false)
     {
          worker_thread_ = boost::thread(&net_worker::run, this);
     }
     
     ~net_worker()
     {
         //  delete  ses_;
         //_workerService->post(boost::bind(&boost::asio::io_service::stop, _workerService));
         worker_thread_.join();
     }

     boost::asio::io_service* GetService()
     {
         return worker_service_;
     }

     void done()
     {
         done_ = true;
     }

     void send(void const* data, uint32_t size)
     {
         worker_service_->post(boost::bind(&net_worker::do_send, this, 0, binary::make_bytes_ptr(data, size)));
     }
     
     void send (binary::bytes_cref bytes)
     {
         worker_service_->post(boost::bind(&net_worker::do_send, this, 0, binary::make_bytes_ptr(&bytes[0], bytes.size())));
     }

     void reset_time(double new_time)
     {
         ses_->reset_time(new_time);
     }

     void set_factor(double factor)
     {
         ses_->set_factor(factor);
     }

private:
     void run()
     {
         async_services_initializer asi(false);

         acc_.reset(new  async_acceptor (mod_peer_, boost::bind(&net_worker::on_accepted, this, _1, _2), tcp_error));
         
         worker_service_ = &(asi.get_service());
         
         boost::asio::io_service::work skwark(asi.get_service());

         if(on_render_)
             render_timer_ = ses_->create_timer ( 1/60.f,  [&](double time)
                                                                 {
                                                                     __main_srvc__->post(boost::bind(on_render_,time));
                                                                 } , 1, false);

         calc_timer_   = ses_->create_timer ( period_, boost::bind(&net_worker::on_timer, this ,_1) , 1, false);

         
		 boost::system::error_code ec;
         size_t ret = worker_service_->run(ec);

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

         sockets_[id]->send(binary::raw_ptr(*data), size);
         if (ec)
         {
             LogError("TCP send error: " << ec.message());
             return;
         }

     }

private:

     void on_timer(double time)
     {
         if(done_)
         {
             ses_->stop();
             return;
         }

         on_update_(time);
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
     }
     
     void do_receive(binary::bytes_ptr data/*, endpoint const& peer*/) 
     {
         if (on_receive_)
             on_receive_(binary::raw_ptr(*data), binary::size(*data)/*, peer*/);
     }

     void on_disconnected(error_code const& ec)      
     {
         LogInfo("Client " << " disconnected with error: " << ec.message() );
         delete  ses_;
         worker_service_->post(boost::bind(&boost::asio::io_service::stop, worker_service_));
         //_workerThread.join();
     }

     void on_disconnected(boost::system::error_code const& ec, uint32_t sock_id)
     {  
         LogInfo("Client " << sock_id << " disconnected with error: " << ec.message() );
         sockets_.erase(sock_id);
         // peers_.erase(std::find_if(peers_.begin(), peers_.end(), [sock_id](std::pair<id_type, uint32_t> p) { return p.second == sock_id; }));
         // delete  ses_;
         worker_service_->post(boost::bind(&boost::asio::io_service::stop, worker_service_));
         delete  ses_;
         __main_srvc__->post(boost::bind(&boost::asio::io_service::stop, __main_srvc__));
         // _workerThread.join();
     }

     void on_error(boost::system::error_code const& ec, uint32_t sock_id)
     {
         LogError("TCP error: " << ec.message());
         sockets_.erase(sock_id);
         // peers_.erase(std::find_if(peers_.begin(), peers_.end(), [sock_id](std::pair<id_type, uint32_t> p) { return p.second == sock_id; }));
     }

private:
    boost::scoped_ptr<async_acceptor>                           acc_   ;
    std::map<uint32_t, std::shared_ptr<tcp_fragment_wrapper> >  sockets_;
private:
    boost::thread                                               worker_thread_;
    boost::asio::io_service*                                    worker_service_;
    std::shared_ptr<boost::asio::io_service::work>              work_;
    const  endpoint                                             mod_peer_;


private:
    on_receive_f                                                on_receive_;
    on_update_f                                                 on_update_;
    on_update_f                                                 on_render_;

private:
    bool                                                        done_;

private:
    double                                                      period_;

private:
    net::timer_connection                                       render_timer_;
    net::timer_connection                                       calc_timer_ ;
    net::timer_connection                                       ctrl_timer_;
    net::ses_srv*                                               ses_;
};


#define MULTITHREADED

#ifndef  MULTITHREADED
struct visapp
{
    typedef boost::function<void(run const& msg)>                   on_run_f;
    typedef boost::function<void(container_msg const& msg)>   on_container_f;

    visapp(endpoint peer, kernel::vis_sys_props const& props/*, binary::bytes_cref bytes*/,int argc, char** argv)
        : systems_  (get_systems())
	    , osg_vis_  (CreateVisual())
        , vis_sys_  (create_vis(props/*, bytes*/))
        , ctrl_sys_ (get_systems()->get_control_sys(),0.003/*cfg().model_params.csys_step*/)
        , mod_sys_  (get_systems()->get_model_sys(),0.003/*cfg().model_params.msys_step*/)
       
    {   

        disp_
            .add<setup_msg                 >(boost::bind(&visapp::on_setup      , this, _1))
            .add<container_msg         >(boost::bind(&visapp::on_container  , this, _1))
            .add<create                >(boost::bind(&visapp::on_create     , this, _1))
            ;
       


        w_.reset (new net_worker( peer 
            , boost::bind(&msg_dispatcher<uint32_t>::dispatch, &disp_, _1, _2, 0/*, id*/)
            , boost::bind(&visapp::update, this, _1)
            , boost::bind(&visapp::on_render, this, _1)
            ));

		w_->set_factor(1.0);
    }

    ~visapp()
    {
	   w_.reset();
       vis_sys_.reset();
    }

private:
    
    void on_render(double time)
    {   
        high_res_timer                _hr_timer;
		systems_->update_vis_messages();
        vis_sys_.update(time);
        osg_vis_->Render();
        //LogInfo( "on_render(double time)" << _hr_timer.get_delta());

    }
                              
    void update(double time)
    {   
        double sim_time = osg_vis_->GetInternalTime();

        if(sim_time  < 0)
        {
            w_->done();
           __main_srvc__->stop();
           return;
        }
        else
        {
		FIXME(И эту хрень тоже надо перенести сервис);
		   systems_->update_messages();
           mod_sys_.update(time);
           ctrl_sys_.update(time);
        }

    }
    
   
    void on_setup(setup_msg const& msg)
    {
         create_objects(msg.icao_code);
         osg_vis_->EndSceneCreation();

         binary::bytes_t bts =  std::move(wrap_msg(ready_msg(0)));
         w_->send(&bts[0], bts.size());

		 
        // LogInfo("Got setup message: " << msg.srv_time << " : " << msg.task_time );
    }

    void on_container(container_msg const& msg)
    {
        for (size_t i = 0; i < msg.msgs.size(); ++i)
            disp_.dispatch_bytes(msg.msgs[i]);
    }

    void on_create(create const& msg)
    {
        kernel::object_info_ptr  a;
        
        if(auto fp = fn_reg::function<kernel::object_info_ptr (create const&)>(kernel::system* csys, "create_object"))
            a = fp(ctrl_sys_.get(), msg);

        //LogInfo("Got create message: " << msg.model_name << "   " << msg.object_kind << "   " << msg.course << " : " << msg.lat << " : " << msg.lon  );
    }



    void create_objects(const std::string& airport)
    {
        using namespace binary;
        using namespace kernel;

        high_res_timer hr_timer;

        get_systems()->create_auto_objects();
        
        force_log fl;       
        LOG_ODS_MSG( "create_objects(const std::string& airport): create_auto_objects() " << hr_timer.get_delta() << "\n");
        
        if(auto fp = fn_reg::function<void(const std::string&)>("create_objects"))
            fp(airport);

        force_log fl2;  
        LOG_ODS_MSG( "create_objects(const std::string& airport): create_objects " << hr_timer.get_delta() << "\n");

#if 1
        kernel::object_info_ptr reg_obj = find_object<object_info_ptr>(dynamic_cast<kernel::object_collection*>(ctrl_sys_.get_sys().get()),"objects_reg") ;   
        
        if (reg_obj)
        {
            void (objects_reg::control::*on_run)       (net_layer::msg::run_msg const& msg)           = &objects_reg::control::inject_msg;

			disp_
                .add<run                   >(boost::bind(on_run , objects_reg::control_ptr(reg_obj).get(), _1));
        }
#endif
    }

    kernel::visual_system_ptr create_vis(kernel::vis_sys_props const& props, av::IVisual* vis/*, binary::bytes_cref bytes*/)
    {
        using namespace binary;
        using namespace kernel;

        //systems_factory_ptr sys_fab = nfi::function<systems_factory_ptr()>("systems", "create_system_factory")();
        auto ptr = // /*sys_fab->*/create_visual_system(msg_service_, /*vw_->get_victory(),*/ props);
                   get_systems()->get_visual_sys(vis);
        //dict_t dic;
        //binary::unwrap(bytes, dic);

        //system_ptr(ptr)->load_exercise(dic);
        return ptr;
    }

private:
    msg_dispatcher<uint32_t>                                       disp_;

private:
	systems_ptr                                                 systems_;

    updater                                                     mod_sys_;
    updater                                                    ctrl_sys_;
    updater                                                     vis_sys_;
    IVisual*                                                    osg_vis_;

private:
    boost::scoped_ptr<net_worker>                                     w_;
};
#else

struct global_timer : boost::noncopyable
{
    inline void set_time(double time)
    {
         internal_time(time);
    }

    inline double get_time()
    {
        return internal_time();
    }

    inline void set_factor(double factor)
    {
        internal_factor(factor);
    }

private:
    
    double internal_time(double time = -1.0)
    {
        static double                 time_    = 0.0;
        static double                 prev_time_    = 0.0;
        static high_res_timer         hr_timer;
        static boost::recursive_mutex guard_;
        boost::lock_guard<boost::recursive_mutex> lock(guard_);

        if(time > 0.0 )
        {
			time_ = cg::eq(time,prev_time_,20e-3)? prev_time_: time;
            hr_timer = high_res_timer();
            prev_time_ = time_;
            return time_;
        }

        if(internal_factor()>0.0)
            prev_time_ = time_ + hr_timer.get_delta();
        
        return  prev_time_;
    }

    double internal_factor(double factor=-1.0)
    {
        static double   factor_ = 0.0; 
        static boost::recursive_mutex guard_;
        boost::lock_guard<boost::recursive_mutex> lock(guard_);

        if(factor >= 0.0 )
        {
            factor_ = factor;
        }

        return factor_;
    }

};

struct visapp_impl
{
    friend struct visapp;

    visapp_impl(kernel::vis_sys_props const& props/*, binary::bytes_cref bytes*/)
        : systems_  (get_systems())
        , osg_vis_  (av::CreateVisual())
        , vis_sys_  (create_vis(props, osg_vis_/*, bytes*/))
    {   

    }

    ~visapp_impl()
    {
        vis_sys_.reset();
    }

    bool done()
    {
        return osg_vis_->Done();
    }

    void end_this()
    {
        osg_vis_->EndSceneCreation();
    }

private:

    void on_render(double time)
    {   
#if 0
		force_log fl;       
		LOG_ODS_MSG( "on_render(double time)= " << time  <<"\n");
#endif

		systems_->update_vis_messages();
        vis_sys_.update(time);
        osg_vis_->Render(time);
    }

private:

    kernel::visual_system_ptr create_vis(kernel::vis_sys_props const& props, av::IVisual* vis/*, binary::bytes_cref bytes*/)
    {
        using namespace binary;
        using namespace kernel;

        auto ptr = systems_->get_visual_sys(vis);

        return ptr;
    }

private:

    systems_ptr                                                 systems_;
    av::IVisualPtr                                              osg_vis_;
    updater                                                     vis_sys_;


};

struct visapp
{
    visapp( kernel::vis_sys_props const& props/*, binary::bytes_cref bytes*/)
        : done_  (false)
        , props_ (props)
        , thread_(new boost::thread(boost::bind(&visapp::run, this)))
    {

    }

    ~visapp()
    {
        done();
        thread_->join();
    }

    void run()
    {   
        visapp_impl  va( props_);
        end_of_load_  = boost::bind(&visapp_impl::end_this,&va);
        while (!va.done() && !done_)
          va.on_render(gt_.get_time());
    }
    
    void end_this()
    {
        if(end_of_load_)
            end_of_load_();
    }

    void done()
    {
        done_ = true;
    }

private:
    bool                                 done_;
private:
    kernel::vis_sys_props               props_;
    std::unique_ptr<boost::thread>     thread_;
    boost::function<void()>       end_of_load_;
private:

    global_timer                           gt_;

};



struct mod_app
{
    typedef boost::function<void(run_msg const& msg)>                   on_run_f;
    typedef boost::function<void(container_msg const& msg)>   on_container_f;

    mod_app(endpoint peer, boost::function<void()> eol/*,  binary::bytes_cref bytes*/)
        : systems_  (get_systems())
        , ctrl_sys_ (systems_->get_control_sys())
        , mod_sys_  (systems_->get_model_sys  ())
        , end_of_load_(eol)
        , disp_     (boost::bind(&mod_app::inject_msg      , this, _1, _2)) 
    {   

        disp_
            .add<setup_msg                 >(boost::bind(&mod_app::on_setup      , this, _1))
            .add<state_msg                 >(boost::bind(&mod_app::on_state      , this, _1))
            ;



        w_.reset (new net_worker( peer 
            , boost::bind(&msg_dispatcher<uint32_t>::dispatch, &disp_, _1, _2, 0/*, id*/)
            , boost::bind(&mod_app::update, this, _1)
            , net_worker::on_update_f() /*boost::bind(&mod_app::on_timer, this, _1)*/
            ));


    }

    ~mod_app()
    {
        w_.reset();
    }

private:

    void on_timer(double time)
    {   
    }

    void update(double time)
    {   
        gt_.set_time(time);
		double dt = _hr_timer.set_point();

#if 0
		force_log fl;       
		LOG_ODS_MSG( "void update(double time) " << time << "   dt=  "<< dt << "\n");
#endif
        systems_->update_messages();
        mod_sys_->update(time);
        ctrl_sys_->update(time);
    }


    void on_setup(setup_msg const& msg)
    {
        w_->set_factor(0.0);
        gt_.set_factor(0.0);

        create_objects(/*msg.icao_code*/msg);
        
        if(end_of_load_)
            end_of_load_(); 

        binary::bytes_t bts =  std::move(wrap_msg(ready_msg(0)));
        w_->send(&bts[0], bts.size());
    }

    void on_state(state_msg const& msg)
    {
       w_->set_factor(msg.factor);
       w_->reset_time(msg.srv_time / 1000.0f);
       gt_.set_factor(msg.factor);
    }

#if 0
    void on_create(create_msg const& msg)
    {
		reg_obj_->create_object(msg);

        LogInfo("Got create message: " << msg.model_name << "   " << (short)msg.object_kind << "   "<< msg.orien.get_course() << " : " << msg.pos.x << " : " << msg.pos.y  );

    }
#endif


    void inject_msg(const void* data, size_t size)
    {
        if (reg_obj_)
        {
            reg_obj_->inject_msg(data, size);
        }
    }

    void create_objects(const std::string& airport)
    {
        using namespace binary;
        using namespace kernel;

        high_res_timer hr_timer;

        systems_->create_auto_objects();

        force_log fl;       
        LOG_ODS_MSG( "create_objects(const std::string& airport): create_auto_objects() " << hr_timer.set_point() << "\n");

        if(auto fp = fn_reg::function<void(const std::string&)>("create_objects"))
            fp(airport);

        force_log fl2;  
        LOG_ODS_MSG( "create_objects(const std::string& airport): create_objects " << hr_timer.set_point() << "\n");


        reg_obj_ = objects_reg::control_ptr(find_object<object_info_ptr>(dynamic_cast<kernel::object_collection*>(ctrl_sys_.get()),"aircraft_reg")) ;   

        if (reg_obj_)
        {
            void (net_worker::*send_)       (binary::bytes_cref bytes)              = &net_worker::send;

            reg_obj_->set_sender(boost::bind(send_, w_.get(), _1 ));
        }

    }

    void create_objects(const setup_msg& msg)
    {
        using namespace binary;
        using namespace kernel;

        dict_t dict;
        if(auto fp = fn_reg::function<void(const setup_msg&, dict_t& dict)>("pack_objects"))
            fp(msg, dict);

        reg_obj_ = objects_reg::control_ptr(find_object<object_info_ptr>(dynamic_cast<kernel::object_collection*>(ctrl_sys_.get()),"aircraft_reg")) ;   

        if (reg_obj_)
        {
            void (net_worker::*send_)       (binary::bytes_cref bytes)              = &net_worker::send; // &net_worker::send_proxy;

            reg_obj_->set_sender(boost::bind(send_, w_.get(), _1 ));
        }

#if 0
        binary::bytes_t bts =  std::move(network::wrap_msg(create_session(std::string("session_name"), binary::wrap(dict),0.0)));
        w_->send_session_clients(bts);
#endif

        mod_sys_->load_exercise(dict);

    }

private:
    systems_ptr                                                 systems_;
    kernel::system_ptr                                          mod_sys_;
    kernel::system_ptr                                         ctrl_sys_;

private:
    msg_dispatcher<uint32_t>                                       disp_;

private:
    on_container_f                                              on_cont_;
    boost::function<void()>                                 end_of_load_;
private:
    boost::scoped_ptr<net_worker>                                     w_;

private:
    objects_reg::control_ptr                                   reg_obj_;

    global_timer                                                     gt_;
	high_res_timer                                             _hr_timer;
};
#endif


}

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

        kernel::vis_sys_props props;
        props.base_point = ::get_base();

#ifdef  MULTITHREADED
        visapp  va(props);
        mod_app ma(peer,boost::bind(&visapp::end_this,&va));
#else 
		visapp  va(peer,props,argc, argv);
#endif
        boost::system::error_code ec;
        __main_srvc__->run(ec);

    }
    catch(verify_error const& err)
    {
        std::string serr(err.what());
        LogInfo( serr );
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
