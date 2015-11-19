#include "stdafx.h"

#include "async_services/async_services.h"
#include "network/msg_dispatcher.h"
#include "kernel/systems.h"
#include "kernel/systems/systems_base.h"
#include "kernel/systems/fake_system.h"
#include "kernel/object_class.h"
#include "kernel/msg_proxy.h"
#include "common/test_msgs.h"
#include "common/locks.h"
#include "common/time_counter.h"

#include "kernel/systems/vis_system.h"

#include "utils/high_res_timer.h"

#include "av/Visual.h"
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

    
namespace details
{
    struct timer_holder { virtual ~timer_holder(){} };

    struct session /*: net_layer::ses_srv*/ {
        
        session ( double initial_time )
            : time_(initial_time)
        {
            time_.set_factor(1.0);
        }
        
        double time() const
        {
           return time_.time();
        }

        void stop()
        {
            time_reset_signal_(/*ses_time*/time(), 0);
        }
        
        void set_factor(double factor)
        {
        }
        
        void send(binary::bytes_cref bytes, bool sure)
        {
            //Assert(send_);
            //send_(*network::wrap(data_msg(bytes)), sure);
        }

        void reset_time(double new_time)
        {
        }

        bool local() const
        {
            return true/*control_*/;
        }
        
        void session_data_loaded()
        {
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

    private:
        time_control time_;


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
         , ses_        (new details::session(0))
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

private:
     void run()
     {
         async_services_initializer asi(false);

         acc_.reset(new  async_acceptor (_peer, boost::bind(&net_worker::on_accepted, this, _1, _2), tcp_error));
         
         _workerService = &(asi.get_service());
         
         boost::asio::io_service::work skwark(asi.get_service());

         
         render_timer_ = session_timer::create (ses_, 1/60.f,  [&](double time)
                                                             {
                                                                 __main_srvc__->post(boost::bind(on_render_,time));
                                                             } , 1, false);

         calc_timer_   = session_timer::create (ses_, 0.05f, boost::bind(&net_worker::on_timer, this ,_1) , 1, false);

         
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
    //async_timer                                                   timer_; 

    bool                                                          done_;

private:
    double                                                       period_;

private:
    session_timer::connection                                 render_timer_;
    session_timer::connection                                 calc_timer_ ;
    session_timer::connection                                 ctrl_timer_;
    details::session*                                         ses_;
};


struct visapp
{
    typedef boost::function<void(run const& msg)>   on_run_f;

    visapp(endpoint peer, kernel::vis_sys_props const& props/*, binary::bytes_cref bytes*/,int argc, char** argv)
        : osg_vis_  (CreateVisual())
        , vis_sys_  (create_vis(props/*, bytes*/))
        , ctrl_sys_ (sys_creator()->get_control_sys(),0.03/*cfg().model_params.csys_step*/)
        , mod_sys_  (sys_creator()->get_model_sys(),0.03/*cfg().model_params.msys_step*/)
       
    {   

        disp_
            .add<setup                 >(boost::bind(&visapp::on_setup      , this, _1))
            .add<run                   >(boost::bind(&visapp::on_run        , this, _1))
            .add<create                >(boost::bind(&visapp::on_create     , this, _1))
            ;
       


        w_.reset (new net_worker( peer 
            , boost::bind(&msg_dispatcher<uint32_t>::dispatch, &disp_, _1, _2, 0/*, id*/)
            , boost::bind(&visapp::update, this, _1)
            , boost::bind(&visapp::on_render, this, _1)
            ));


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

         LogInfo("Got setup message: " << msg.srv_time << " : " << msg.task_time );
    }

    void on_run(run const& msg)
    {
        if(on_run_)
            on_run_(msg);
        LogInfo("Got run message: " << msg.speed << " : " << msg.time );
    }
    
    void async_run(run const& msg)
    {
        LogInfo("async_run got run message: "  );
    }

    void on_create(create const& msg)
    {
        auto fp = fn_reg::function<kernel::object_info_ptr (create const&)>("create_aircraft");
        kernel::object_info_ptr  a;
        
        if(fp)
            a = fp(msg);

        LogInfo("Got create message: " << msg.course << " : " << msg.lat << " : " << msg.lon  );
    }


    uint32_t next_id()
    {
        static uint32_t id = 0;
        return id++;
    }

    void create_objects(const std::string& airport)
    {
        using namespace binary;
        using namespace kernel;

        high_res_timer hr_timer;

        sys_creator()->create_auto_objects();
        
        force_log fl;       
        LOG_ODS_MSG( "create_objects(const std::string& airport): create_auto_objects() " << hr_timer.get_delta() << "\n");
        
        auto fp = fn_reg::function<void(const std::string&)>("create_objects");
        if(fp)
            fp(airport);

        force_log fl2;  
        LOG_ODS_MSG( "create_objects(const std::string& airport): create_objects " << hr_timer.get_delta() << "\n");
        
        kernel::object_info_ptr reg_obj = find_object<object_info_ptr>(dynamic_cast<kernel::object_collection*>(ctrl_sys_.get_sys().get()),"aircraft_reg") ;   
        
        if (reg_obj)
        {
            on_run_ = (boost::bind(&aircraft_reg::control::inject_msg , aircraft_reg::control_ptr(reg_obj).get(), _1));
        }
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
    msg_dispatcher<uint32_t>                                       disp_;

private:
    updater                                                     mod_sys_;
    updater                                                    ctrl_sys_;
    updater                                                     vis_sys_;
    IVisual*                                                    osg_vis_;
private:
    on_run_f                                                    on_run_;
private:
    boost::scoped_ptr<net_worker>                                     w_;
};


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

        kernel::vis_sys_props props_;
        props_.base_point = ::get_base();

        visapp s(peer, props_ , argc, argv);
        
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
