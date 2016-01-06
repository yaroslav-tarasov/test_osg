#include "stdafx.h"

#include "av/Visual.h"

#include "async_services/async_services.h"
#include "network/msg_dispatcher.h"
#include "kernel/systems.h"
#include "kernel/systems/systems_base.h"
#include "kernel/systems/fake_system.h"
#include "kernel/object_class.h"
#include "kernel/msg_proxy.h"
#include "kernel/systems/vis_system.h"
#include "common/test_msgs.h"
#include "common/locks.h"
#include "common/time_counter.h"
#include "utils/high_res_timer.h"
#include "tests/systems/test_systems.h"
#include "objects/registrator.h"

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

struct time_control
{
    time_control(double initial_time = 0.);

    void   set_factor(double factor, optional<double> ext_time = boost::none);
    double get_factor() const;
    double time      () const;

private:
    double          last_time_;
    double          time_factor_;
    time_counter    time_flow_;
};

/////////////////////////////////////////////////////////
// time_control implementation
inline time_control::time_control(double initial_time)
    : last_time_    (initial_time)
    , time_factor_  (0)
{
}

inline void time_control::set_factor(double factor, optional<double> ext_time)
{
    last_time_   = ext_time ? *ext_time : time();    
    time_factor_ = factor;
    time_flow_.reset();
}

inline double time_control::get_factor() const
{
    return time_factor_;
}

inline double time_control::time() const
{
    return last_time_ + time_counter::to_double(time_flow_.time()) * time_factor_;
}

enum messages_id
{
    id_task_loaded_session, // sent from client to session_controller, indicating all session data successfully loaded by task
    id_session_loaded     , // sent from session_controller to all clients, indicating all clients are ready for session processing

    id_reset_time_request,
    id_new_time_response ,
    id_data_msg    
};

struct new_time_response
    : network::msg_id<id_new_time_response>
{
    new_time_response(double srv_time, double ses_time, double factor)
        : srv_time  (srv_time)
        , ses_time  (ses_time)
        , factor    (factor)
    {
    }

    new_time_response(){}

    double srv_time;
    double ses_time;
    double factor;
};

namespace details
{
    struct timer_holder { virtual ~timer_holder(){} };

    struct session /*: net_layer::ses_srv*/ {
        
        typedef boost::function<double()>           time_func_f;

        session ( time_func_f const& srv_time, double initial_time )
            : time_(initial_time)
            , srv_time_ (srv_time)
        {
            time_.set_factor(0.0);
        }
        

        // native
        double time() const
        {
           return time_.time();
        }

        // фантазии
        void stop()
        {
            time_reset_signal_(/*ses_time*/time(), 0);
        }


        
        // native not realized (not used?)
        void send(binary::bytes_cref bytes, bool sure)
        {
            //Assert(send_);
            //send_(*network::wrap(data_msg(bytes)), sure);
        }

        // modified
        void reset_time(double new_time)
        {
            // сообщение в сеть с новым временем и текущим фактором
            
            time_.set_factor(time_.get_factor(), new_time);
            time_reset_signal_(new_time, time_.get_factor());
        }
        
        // modified
        void set_factor(double factor)
        {
             // сообщение в сеть с новым фактором только, время не устанавливается

            time_.set_factor(factor);
        }

        // modified   
        bool local() const
        {
            return true/*control_*/;
        }

        // native
        void session_data_loaded()
        {
            session_loaded_signal_();
        }
        
        void on_new_time_response(new_time_response const& msg)
        {
            double ses_time = std::max(0., msg.ses_time + (srv_time_() - msg.srv_time) * msg.factor);

            time_.set_factor(msg.factor, ses_time);
            time_reset_signal_(ses_time, msg.factor);
        }

        //net_layer::timer_connection create_timer(double period_sec, on_timer_f const& on_timer, bool adjust_time_factor, bool terminal)
        //{
        //        return net_layer::timer_connection();
        //}

        //net_layer::net_srv&            net_server  () const 
        //{
        //       return net_layer::net_srv();
        //}
        
        ~session()
        {
            session_stopped_signal_();
        }

        DECLARE_EVENT(time_reset    , (double /*time*/, double /*factor*/));
        DECLARE_EVENT(session_stopped   ,   ());
        DECLARE_EVENT(session_loaded, ());

    private:
        time_func_f                     srv_time_;

    private:
        time_control                    time_;



    };

} // details

struct session_timer 
    : details::timer_holder
{
    typedef boost::shared_ptr<details::timer_holder> connection;

    typedef boost::function<void (double /*time*/)> on_timer_f;

    typedef boost::function<void()>                         timer_callback_f;
    typedef boost::function<void(timer_callback_f const&)>  deferred_call_f;
    
    static connection create (details::session* ses,double period_sec, on_timer_f const& on_timer, bool adjust_time_factor, bool terminal = false)
    {
        return connection(
            new session_timer(
            ses, 
            on_timer, 
            period_sec, 
            1/*time_.get_factor()*/, 
            adjust_time_factor, 
            /*terminal ? dfrd_call_ :*/ deferred_call_f()));
    }

    session_timer( details::session* session, on_timer_f const& on_timer, double period, double factor, bool adjust_time_factor, deferred_call_f const& def_call)
        : session_  (session)
        , on_timer_ (on_timer)
        , period_   (period)
        , factor_   (factor)

        , last_time_point_   (session->time())
        , adjust_time_factor_(adjust_time_factor)
        , timer_             (bind(&session_timer::on_timer, this))
        , deferred_call_     (def_call)
    {
        Assert(!cg::eq_zero(period_));

        changing_time_factor_   = session_->subscribe_time_reset     (bind(&session_timer::set_factor, this, _2));
        stopped_session_        = session_->subscribe_session_stopped(bind(&session_timer::session_stopped, this));

        set_next_time_point();
    }

private:
    double get_factor()
    {
        return factor_ ;
    }

    void set_factor(double factor)
    {
        factor_ = factor;
        set_next_time_point();
    }

    void session_stopped()
    {
        session_ = nullptr; // just for check, on_timer must not be invoked after timer_ cancellation 
        timer_.cancel();
    }

private:
    void on_timer()
    {
        static bool inside_timer = false;

        optional<double> on_timer_time;

        if (cg::eq_zero(factor_)) 
        {
            Assert(!adjust_time_factor_);
            on_timer_time = std::floor(session_->time() / period_) * period_;
        }
        else
        {
            double period     = adjust_time_factor_ ? period_ : period_ * factor_;
            double time_point = std::floor(session_->time() / period) * period;

            if (!cg::eq(time_point, last_time_point_))
            {
                on_timer_time    = time_point;
                last_time_point_ = time_point;
            }
        }

        set_next_time_point();

        // must be the last call (!) on timer callback
        if (!inside_timer && on_timer_time)
        {
            locks::bool_lock bl(inside_timer); 

            if (deferred_call_) // terminal timer 
            {
                timer_.cancel();
                deferred_call_(boost::bind(on_timer_, *on_timer_time));
            }
            else 
                on_timer_(*on_timer_time);
        }
    }

    void set_next_time_point() 
    {
        if (cg::eq_zero(factor_)) 
        {
            if (!adjust_time_factor_)
                timer_.wait(boost::posix_time::microseconds(int64_t(1e6 * period_)));
            else
                timer_.cancel(); // paused

            return;
        }
		

        double period          = adjust_time_factor_ ? period_ : period_ * factor_;
        double next_time_point = (std::floor(session_->time() / period) + 1) * period;
        double real_delta      = (next_time_point - session_->time()) / factor_;

        timer_.wait(boost::posix_time::microseconds(int64_t(1e6 * real_delta)));
    }

private:
    details::session*   session_;
    on_timer_f          on_timer_;
    double              period_;
    double              factor_;
    double              last_time_point_;
    bool                adjust_time_factor_;
    async_timer         timer_;

private:
    scoped_connection   changing_time_factor_;
    scoped_connection   stopped_session_;

private:
    deferred_call_f     deferred_call_;


};

namespace
{


struct net_worker
{
     typedef boost::function<void(const void* data, size_t size)>   on_receive_f;
     typedef boost::function<void(double time)>                     on_update_f;     
     

     net_worker(const  endpoint &  peer, on_receive_f on_recv , on_update_f on_update, on_update_f on_render)
         : period_     (/*cfg().model_params.msys_step*/0.05)
         , ses_        (new details::session(details::session::time_func_f(),0))
         , _peer       (peer)
         , on_receive_ (on_recv)
         , on_update_  (on_update)
         , on_render_  (on_render)
         , done_       (false)
     {
          _workerThread = boost::thread(&net_worker::run, this);
     }
     
     ~net_worker()
     {
         //  delete  ses_;
         //_workerService->post(boost::bind(&boost::asio::io_service::stop, _workerService));
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
     
     void send (binary::bytes_cref bytes)
     {
         _workerService->post(boost::bind(&net_worker::do_send, this, 0, binary::make_bytes_ptr(&bytes[0], bytes.size())));
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

         acc_.reset(new  async_acceptor (_peer, boost::bind(&net_worker::on_accepted, this, _1, _2), tcp_error));
         
         _workerService = &(asi.get_service());
         
         boost::asio::io_service::work skwark(asi.get_service());

         if(on_render_)
             render_timer_ = session_timer::create (ses_, 1/60.f,  [&](double time)
                                                                 {
                                                                     __main_srvc__->post(boost::bind(on_render_,time));
                                                                 } , 1, false);

         calc_timer_   = session_timer::create (ses_, period_, boost::bind(&net_worker::on_timer, this ,_1) , 1, false);

         
		 boost::system::error_code ec;
         size_t ret = _workerService->run(ec);

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
         _workerService->post(boost::bind(&boost::asio::io_service::stop, _workerService));
         //_workerThread.join();
     }

     void on_disconnected(boost::system::error_code const& ec, uint32_t sock_id)
     {  
         LogInfo("Client " << sock_id << " disconnected with error: " << ec.message() );
         sockets_.erase(sock_id);
         // peers_.erase(std::find_if(peers_.begin(), peers_.end(), [sock_id](std::pair<id_type, uint32_t> p) { return p.second == sock_id; }));
         // delete  ses_;
         _workerService->post(boost::bind(&boost::asio::io_service::stop, _workerService));
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
    boost::thread                                               _workerThread;
    boost::asio::io_service*                                    _workerService;
    std::shared_ptr<boost::asio::io_service::work>              _work;
    const  endpoint                                             _peer;


private:
    on_receive_f                                                on_receive_;
    on_update_f                                                 on_update_;
    on_update_f                                                 on_render_;

private:
    //async_timer                                                timer_; 

    bool                                                         done_;

private:
    double                                                       period_;

private:
    session_timer::connection                                 render_timer_;
    session_timer::connection                                 calc_timer_ ;
    session_timer::connection                                 ctrl_timer_;
    details::session*                                         ses_;
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
            .add<setup                 >(boost::bind(&visapp::on_setup      , this, _1))
            //.add<run                   >(boost::bind(&visapp::on_run        , this, _1))
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
    
   
    void on_setup(setup const& msg)
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
        auto fp = fn_reg::function<kernel::object_info_ptr (create const&)>(kernel::system* csys, "create_object");
        kernel::object_info_ptr  a;
        
        if(fp)
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
        
        auto fp = fn_reg::function<void(const std::string&)>("create_objects");
        if(fp)
            fp(airport);

        force_log fl2;  
        LOG_ODS_MSG( "create_objects(const std::string& airport): create_objects " << hr_timer.get_delta() << "\n");

#if 1
        kernel::object_info_ptr reg_obj = find_object<object_info_ptr>(dynamic_cast<kernel::object_collection*>(ctrl_sys_.get_sys().get()),"objects_reg") ;   
        
        if (reg_obj)
        {
            // on_run_ = (boost::bind(&objects_reg::control::inject_msg , objects_reg::control_ptr(reg_obj).get(), _1));
            void (objects_reg::control::*on_run)       (net_layer::msg::run const& msg)           = &objects_reg::control::inject_msg;

			disp_
                .add<run                   >(boost::bind(on_run , objects_reg::control_ptr(reg_obj).get(), _1));
        }
#endif
    }

    kernel::visual_system_ptr create_vis(kernel::vis_sys_props const& props/*, binary::bytes_cref bytes*/)
    {
        using namespace binary;
        using namespace kernel;

        //systems_factory_ptr sys_fab = nfi::function<systems_factory_ptr()>("systems", "create_system_factory")();
        auto ptr = // /*sys_fab->*/create_visual_system(msg_service_, /*vw_->get_victory(),*/ props);
                   get_systems()->get_visual_sys();
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
        , osg_vis_  (CreateVisual())
        , vis_sys_  (create_vis(props/*, bytes*/))
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

    kernel::visual_system_ptr create_vis(kernel::vis_sys_props const& props/*, binary::bytes_cref bytes*/)
    {
        using namespace binary;
        using namespace kernel;

        auto ptr = systems_->get_visual_sys();

        return ptr;
    }

private:

    systems_ptr                                                 systems_;
    updater                                                     vis_sys_;
    IVisual*                                                    osg_vis_;

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
    typedef boost::function<void(run const& msg)>                   on_run_f;
    typedef boost::function<void(container_msg const& msg)>   on_container_f;

    mod_app(endpoint peer, boost::function<void()> eol/*,  binary::bytes_cref bytes*/)
        : systems_  (get_systems())
        , ctrl_sys_ (systems_->get_control_sys()/*,0.02*//*cfg().model_params.csys_step*/)
        , mod_sys_  (systems_->get_model_sys  ()/*,0.02*//*cfg().model_params.msys_step*/)
        , end_of_load_(eol)
        , disp_     (boost::bind(&mod_app::inject_msg      , this, _1, _2)) 
    {   

        disp_
            .add<setup                 >(boost::bind(&mod_app::on_setup      , this, _1))
            .add<create                >(boost::bind(&mod_app::on_create     , this, _1))
            .add<state                 >(boost::bind(&mod_app::on_state      , this, _1))
            ;



        w_.reset (new net_worker( peer 
            , boost::bind(&msg_dispatcher<uint32_t>::dispatch, &disp_, _1, _2, 0/*, id*/)
            , boost::bind(&mod_app::update, this, _1)
            , boost::bind(&mod_app::on_timer, this, _1)
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


    void on_setup(setup const& msg)
    {
        w_->set_factor(0.0);
        gt_.set_factor(0.0);

        create_objects(msg.icao_code);
        
        end_of_load_();  //osg_vis_->EndSceneCreation();

        binary::bytes_t bts =  std::move(wrap_msg(ready_msg(0)));
        w_->send(&bts[0], bts.size());
    }

    void on_state(state const& msg)
    {
       w_->set_factor(msg.factor);
       w_->reset_time(msg.srv_time / 1000.0f);
       gt_.set_factor(msg.factor);
    }

    void on_create(create const& msg)
    {
		reg_obj_->create_object(msg);

        LogInfo("Got create message: " << msg.model_name << "   " << (short)msg.object_kind << "   "<< msg.orien.get_course() << " : " << msg.pos.x << " : " << msg.pos.y  );

    }


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

        auto fp = fn_reg::function<void(const std::string&)>("create_objects");
        if(fp)
            fp(airport);

        force_log fl2;  
        LOG_ODS_MSG( "create_objects(const std::string& airport): create_objects " << hr_timer.set_point() << "\n");


        reg_obj_ = objects_reg::control_ptr(find_object<object_info_ptr>(dynamic_cast<kernel::object_collection*>(ctrl_sys_.get()),"aircraft_reg")) ;   

        if (reg_obj_)
        {
            void (net_worker::*send_)       (binary::bytes_cref bytes)              = &net_worker::send;

            reg_obj_->set_sender(boost::bind(send_, w_.get(), _1 ));

#if 0
            void (mod_app::*on_run)       (net_layer::msg::run const& msg)           = &mod_app::inject_msg;
            void (mod_app::*on_container) (net_layer::msg::container_msg const& msg) = &mod_app::inject_msg;

            disp_
                .add<run                   >(boost::bind(on_run      , this , _1))
                .add<container_msg         >(boost::bind(on_container, this , _1));
#endif

        }

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
