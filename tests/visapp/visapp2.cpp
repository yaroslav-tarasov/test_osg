#include "stdafx.h"


#include "ui/tray_icon.h"

#include "async_services/async_services.h"
#include "network/msg_dispatcher.h"
#include "kernel/systems.h"
#include "kernel/systems/systems_base.h"
#include "kernel/systems/fake_system.h"
#include "kernel/object_class.h"
#include "kernel/msg_proxy.h"
#include "common/ext_msgs.h"
#include "common/locks.h"
#include "common/time_counter.h"


#include "kernel/systems/vis_system.h"

#include "utils/high_res_timer.h"

#include "tests/osg_widget/OSGWidget.h"

#include "av/Visual.h"
#include "tests/systems/factory_systems.h"

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
    
    void stop_request()
    {
        QCoreApplication::instance()->exit(0);
    }

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

        DECLARE_EVENT(time_reset    , (double /*time*/, double /*factor*/));

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
        //stopped_session_        = session_->subscribe_session_stopped(bind(&session_timer::session_stopped, this));

        set_next_time_point();
    }

private:
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
        {
            run();
        }

        ~net_worker()
        {
            acc_.reset();
            sockets_.clear();
       }


        inline void send(void const* data, uint32_t size)
        {
            error_code_t ec;
            sockets_[0]->send(&size, sizeof(uint32_t));
            if (ec)
            {
                LogError("TCP send error: " << ec.message());
                return;
            }

            sockets_[0]->send(data, size);
            if (ec)
            {
                LogError("TCP send error: " << ec.message());
                return;
            }
        }

    private:
        void run()
        {
            acc_.reset(new  async_acceptor (_peer, boost::bind(&net_worker::on_accepted, this, _1, _2), tcp_error));

            render_timer_ = session_timer::create (ses_, 0.01f,  boost::bind(on_render_,_1) , 1, false);
            calc_timer_   = session_timer::create (ses_, period_,  boost::bind(on_update_,_1) , 1, false); 

        }

        uint32_t next_id()
        {
            static uint32_t id = 0;
            return id++;
        }

    private:

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


    private:
        boost::scoped_ptr<async_acceptor>                           acc_   ;
        std::map<uint32_t, std::shared_ptr<tcp_fragment_wrapper> >  sockets_;
    private:
        const  endpoint                                             _peer;


    private:
        on_receive_f                                                on_receive_;
        on_update_f                                                 on_update_;
        on_update_f                                                 on_render_;

    private:
        double                                                      period_;

    private:
        session_timer::connection                                   render_timer_;
        session_timer::connection                                   calc_timer_ ;
        session_timer::connection                                   ctrl_timer_;
        details::session*                                           ses_;
    };

    using namespace visual;

    struct visapp
    {
        visapp(endpoint peer, kernel::vis_sys_props const& props/*, binary::bytes_cref bytes*/,int argc, char** argv)
            : vw_   (visual::create_widget( CUSTOM_GL_WIDGET4 ))
            , qvw_  (dynamic_cast<QWidget*>(vw_.get()))
            , vis_sys_  (create_vis(props/*, bytes*/))
            , ctrl_sys_ (sys_creator()->get_control_sys(),0.03/*cfg().model_params.csys_step*/)
            , mod_sys_  (sys_creator()->get_model_sys(),0.03/*cfg().model_params.msys_step*/)

        {   
            
		    

            disp_
                .add<setup                 >(boost::bind(&visapp::on_setup      , this, _1))
                .add<run                   >(boost::bind(&visapp::on_run        , this, _1))
                .add<create                >(boost::bind(&visapp::on_create     , this, _1))
                ;
#if 1
            w_.reset (new net_worker( peer 
                , boost::bind(&msg_dispatcher<uint32_t>::dispatch, &disp_, _1, _2, 0/*, id*/)
                , boost::bind(&visapp::update, this, _1)
                , boost::bind(&visapp::on_render, this, _1)
                ));
#endif
          
            if(qvw_) 
                qvw_->show();
        }

        ~visapp()
        {
            vis_sys_.reset();
        }

    private:

        void on_render(double time)
        {   
            high_res_timer                _hr_timer;
            vis_sys_.update(time);
			vw_->redraw();

            //force_log fl;
            //LOG_ODS_MSG( "on_render(double time)" << _hr_timer.get_delta() << "\n");
        }

        void update(double time)
        {   
            double sim_time = 0; // osg_vis_->GetInternalTime();

            if(sim_time  < 0)
            {
                __main_srvc__->stop();
                stop_request();
                return;
            }
            else
            {
                mod_sys_.update(time);
                ctrl_sys_.update(time);

            }


            //LogInfo("update ");
        }


        void on_setup(setup const& msg)
        {
            // vw_->createScene();
            create_objects(msg.icao_code);
            vw_->endSceneCreation();

            binary::bytes_t bts =  std::move(wrap_msg(ready_msg(0)));
            w_->send(&bts[0], bts.size());

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
            if(auto fp = fn_reg::function<void(create const&)>("create_aircraft"))
                fp(msg);

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

            sys_creator()->create_auto_objects();

            if(auto fp = fn_reg::function<void(const std::string&)>("create_objects"))
                fp(airport);
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
        // visual::OSGWidget*                                            vw_;
        visual::visual_widget_ptr                                        vw_;
        QWidget*                                                        qvw_;
        updater                                                     mod_sys_;
        updater                                                    ctrl_sys_;
        updater                                                     vis_sys_;


    private:
        boost::scoped_ptr<net_worker>                                     w_;

    };

}

int visapp2( int argc, char** argv )
{
 	logger::need_to_log(/*true*/);

    QApplication app (argc, argv);
    app.setQuitOnLastWindowClosed(false);
    async_services_initializer init(true);
    
    __main_srvc__ = &init.get_service();

    logging::add_console_writer();
    logging::add_default_file_writer();


    //cmd_line::arg_map am;
    //if (!am.parse(cmd_line::naive_parser().add_arg("task_id", true), argc, argv))
    //{
    //    LogError("Invalid command line");
    //    return 1;
    //}

    //optional<binary::size_type> task_id;
    //if (am.contains("task_id")) 
    //    task_id = am.extract<binary::size_type>("task_id");

    try
    {
        endpoint peer(cfg().network.local_address);

        kernel::vis_sys_props props_;
        props_.base_point = ::get_base();

        visapp s(peer, props_ , argc, argv);

        tray_icon tricon(":/resources/projector.png", &app);
        app.connect(&tricon, SIGNAL(menu_exit()), &app, SLOT(quit()));    
        tricon.show();

        return app.exec();
    }
    catch(const boost::filesystem::filesystem_error& e)
    {
        auto c = e.code().message();
        LogError(c);
    }

	return 0; 

}

AUTO_REG(visapp2)