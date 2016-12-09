#include "stdafx.h"

#include "av/Visual.h"

#include "async_services/async_services.h"
#include "network/msg_dispatcher.h"
#include "kernel/systems.h"
#include "kernel/systems/systems_base.h"
#include "kernel/object_class.h"
#include "kernel/msg_proxy.h"
#include "kernel/systems/vis_system.h"
#include "common/ext_msgs.h"
#include "common/ext_msgs2.h"
#include "common/time_counter.h"
#include "utils/high_res_timer.h"
#include "tests/systems/factory_systems.h"
#include "objects/registrator.h"

#include "net_layer/net_worker.h"
#include "net_layer/app_ports.h"

#include <boost/container/map.hpp>

#include "asio2asio_dispatcher.h"
#include "asio2asio_initializer.h"

#ifdef _WIN32
#include  <mmsystem.h>  // timerBeginPeriod
#endif

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
    void tcp_error(error_code const& err)
    {
        LogError("Error happened: " << err);
        		network::asio2asio::disp().call_main(boost::bind(&boost::asio::io_service::stop, &network::asio2asio::disp().get_main_service()));
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
     typedef boost::function<void(const void* data, size_t size)>   on_ses_receive_f;
     typedef boost::function<void(double time)>                     on_update_f;     
     typedef boost::function<void()>                                on_all_connected_f; 
     typedef boost::function<void(const ready_msg&)>                on_client_ready_f; 


     struct ses_helper
     {
         ses_helper (net_worker* nw)
             : nw_ (nw)
         {}

         void init()
         {
              ses_acc_.reset(new  async_acceptor (nw_->peer_, boost::bind(&ses_helper::on_accepted, this, _1, nw_->peer_ ), tcp_error));
         }

         void reset()
         {
             ses_acc_.reset();
             cons_.clear();
             proxy_socket_.reset();
             sockets_.clear();
         }

         void on_accepted(network::tcp::socket& sock, endpoint const& peer)
         {
             const uint32_t id = peer.port;
             proxy_socket_ = std::shared_ptr<tcp_fragment_wrapper>(new tcp_fragment_wrapper( sock 
                 , boost::bind(&ses_helper::on_recieve     , this, _1, _2, peer)
                 , boost::bind(&net_worker::on_disconnected, nw_ , _1, peer)
                 , boost::bind(&net_worker::on_error       , nw_ , _1, peer)
                 ));        

             LogInfo("Client " << peer << " accepted");
         }

         void on_recieve(const void* data, size_t size, endpoint const& peer)
         {
             network::asio2asio::disp().call_main(boost::bind(&ses_helper::do_recieve, this, binary::make_bytes_ptr(data, size) ));
         }

         void do_recieve(binary::bytes_ptr data )
         {
             if (nw_->on_ses_recv_)
                 nw_->on_ses_recv_(binary::raw_ptr(*data), binary::size(*data));
         }
         
         
         void vis_connect(const std::vector<endpoint>& vis_peers)
         {  
             vis_peers_  = vis_peers;
             for (auto it = vis_peers_.begin(); it!= vis_peers_.end(); ++it )
             {

                 (*it).port = net_layer::visapp_ses_port;
                 cons_[*it].reset(  new async_connector(*it, boost::bind(&ses_helper::on_connected, this, _1, _2), [](error_code const& err){}/*boost::bind(&net_worker::disconnect, nw_, _1)*/ , tcp_error));

             }

         }

         void send_clients(binary::bytes_cref data)
         {
             size_t size = binary::size(data);
             error_code_t ec;

             for( auto it = vis_peers_.begin();it!=vis_peers_.end(); ++it )
                 if(sockets_.find(*it)!=sockets_.end()) sockets_[*it]->send(binary::raw_ptr(data), size);

             if (ec)
             {
                 LogError("TCP send error: " << ec.message());
                 return;
             }
         }

		 void send_proxy(binary::bytes_ptr data)
		 {   
			 size_t size = binary::size(*data);
			 error_code_t ec;

			 proxy_socket_->send(binary::raw_ptr(*data), size);
			 if (ec)
			 {
				 LogError("TCP send error: " << ec.message());
				 return;
			 }

		 }

         void on_connected(network::tcp::socket& sock, network::endpoint const& peer)
         {
             LogInfo("Session helper connected to " << peer);

             sockets_[peer] = std::shared_ptr<tcp_fragment_wrapper>(new tcp_fragment_wrapper(
                 sock, boost::bind(&ses_helper::on_recieve, this, _1, _2, peer), [](error_code const& err){tcp_error(err);}/*boost::bind(&net_worker::disconnect, nw_, _1)*/, &tcp_error));  

             nw_->on_all_connected();
         }

         bool all_connected() const
         {
              return vis_peers_.size() == sockets_.size();
         }

         net_worker*                                                                nw_;
         std::shared_ptr<tcp_fragment_wrapper>                            proxy_socket_;

         boost::scoped_ptr<async_acceptor>                                     ses_acc_;
         std::map<network::endpoint, std::shared_ptr<tcp_fragment_wrapper> >   sockets_;

         std::map< endpoint,unique_ptr<async_connector>>                          cons_;
         std::vector<endpoint>                                               vis_peers_;
     };

     net_worker(const  endpoint &  peer,  on_ses_receive_f on_ses_recv , on_receive_f on_recv , on_update_f on_update, on_all_connected_f on_all_connected)
         : period_     (cfg().model_params.msys_step)
         , ses_        (net_layer::create_session(binary::bytes_t(), true))
         , on_ses_recv_(on_ses_recv)
         , on_receive_ (on_recv)
         , on_update_  (on_update)
         , on_all_connected_ (on_all_connected)
         , done_       (false)
         , peer_       (peer)
         , ses_helper_ (this)
     {
          network::asio2asio::disp().call_asio(boost::bind(&net_worker::init, this));
     }
     
     ~net_worker()
     {
         deinit();
         delete  ses_;
     }


     void done()
     {
         done_ = true;
     }

     void send_proxy(void const* data, uint32_t size)
     {
         network::asio2asio::disp().call_asio(boost::bind(&net_worker::do_send_proxy, this, 0, binary::make_bytes_ptr(data, size)));
     }
     
     void send_proxy (binary::bytes_cref bytes)
     {
         network::asio2asio::disp().call_asio(boost::bind(&net_worker::do_send_proxy, this, 0, binary::make_bytes_ptr(&bytes[0], bytes.size())));
     }
      
     void send_clients (binary::bytes_cref bytes)
     {
         network::asio2asio::disp().call_asio(boost::bind(&net_worker::do_send_clients, this, binary::make_bytes_ptr(&bytes[0], bytes.size())));
     }
     
     void send_session_clients (binary::bytes_cref bytes)
     {
         network::asio2asio::disp().call_asio(boost::bind(&net_worker::do_send_session_clients, this, binary::make_bytes_ptr(&bytes[0], bytes.size())));
     }

     void reset_time(double new_time)
     {
         ses_->reset_time(new_time);
     }

     void set_factor(double factor)
     {
         ses_->set_factor(factor);
     }

     void vis_connect(const std::vector<endpoint>& vis_peers)
     {  
         visapp_peers_  = vis_peers;
        
         network::asio2asio::disp().call_asio([this]()
         {
             ses_helper_.vis_connect(visapp_peers_);

             for (auto it = visapp_peers_.begin(); it!= visapp_peers_.end(); ++it )
             {
                 (*it).port = net_layer::visapp_data_port;
                 cons_[*it].reset(  new async_connector(*it, boost::bind(&net_worker::on_connected, this, _1, _2), [](error_code const& err){tcp_error(err);}/*boost::bind(&net_worker::disconnect, this, _1)*/ , tcp_error));
             }
         } );

     }

private:

     void init()
     {
         ses_helper_.init();
         calc_timer_   = ses_->create_timer ( period_, boost::bind(&net_worker::on_timer, this ,_1) , 1, false);
     }

     void deinit()
     {
         ses_helper_.reset();
         calc_timer_.reset();
         sockets_.clear();
     }

     uint32_t next_id()
     {
         static uint32_t id = 0;
         return id++;
     }

     // from struct tcp_connection
     void do_send_proxy(int id, binary::bytes_ptr data)
     {   
			ses_helper_.send_proxy(data);
     }

     void do_send_clients(binary::bytes_ptr data)
     {   
         size_t size = binary::size(*data);
         error_code_t ec;

         for( auto it = visapp_peers_.begin();it!=visapp_peers_.end(); ++it )
            if(sockets_.find(*it)!=sockets_.end()) sockets_[*it]->send(binary::raw_ptr(*data), size);

         if (ec)
         {
             LogError("TCP send error: " << ec.message());
             return;
         }

     }

     void do_send_session_clients(binary::bytes_ptr data)
     {   
          ses_helper_.send_clients(*data);
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
     

#if 0
     void on_accepted(network::tcp::socket& sock, endpoint const& peer)
     {
         if (!srv_)
         {
             srv_ = std::shared_ptr<tcp_fragment_wrapper>(new tcp_fragment_wrapper(
                 sock, boost::bind(&net_worker::on_recieve     , this, _1, _2, peer),
                       boost::bind(&net_worker::on_disconnected, this, _1,     peer),
                       boost::bind(&net_worker::on_error       , this, _1,     peer)));  


            LogInfo("Client " << peer << " accepted");
         }
         else
            LogError("Client " << peer << " rejected. Connection already established");
     }
#endif

     void on_recieve(const void* data, size_t size, endpoint const& peer)
     {
          network::asio2asio::disp().call_main(boost::bind(&net_worker::do_receive, this, binary::make_bytes_ptr(data, size)) );
     }
     
     void do_receive(binary::bytes_ptr data/*, endpoint const& peer*/) 
     {
         if (on_receive_)
             on_receive_(binary::raw_ptr(*data), binary::size(*data)/*, peer*/);
     }

     void on_disconnected(error_code const& ec)      
     {
         LogInfo("Client  disconnected with error: " << ec.message() );
         delete  ses_;
         network::asio2asio::disp().call_asio(boost::bind(&boost::asio::io_service::stop, &network::asio2asio::disp().get_main_service()));
         //_workerThread.join();
     }

     void on_disconnected(boost::system::error_code const& ec, endpoint const& peer)
     {  
         LogInfo("Client " << peer.to_string() << " disconnected with error: " << ec.message() );
         sockets_.erase(peer);
         // peers_.erase(std::find_if(peers_.begin(), peers_.end(), [sock_id](std::pair<id_type, uint32_t> p) { return p.second == sock_id; }));
         // delete  ses_;
         network::asio2asio::disp().call_asio([this]()
         {
             for (auto it = visapp_peers_.begin(); it!= visapp_peers_.end(); ++it )
             {
                 cons_[*it].reset();
             }
         } );

         network::asio2asio::disp().call_asio(boost::bind(&boost::asio::io_service::stop, &network::asio2asio::disp().get_main_service()));
#if 0
         delete  ses_;
#endif
     }

     void on_error(boost::system::error_code const& ec, endpoint const& peer)
     {
         LogError("TCP error: " << ec.message());
         sockets_.erase(peer);
         // peers_.erase(std::find_if(peers_.begin(), peers_.end(), [sock_id](std::pair<id_type, uint32_t> p) { return p.second == sock_id; }));
     }
     
     bool all_connected() const
     {
         return visapp_peers_.size() == sockets_.size();
     }

private:
    
    void on_all_connected()
    {
        if(on_all_connected_ && all_connected() && ses_helper_.all_connected())
            on_all_connected_();
    }

    void on_connected(network::tcp::socket& sock, network::endpoint const& peer)
    {
        LogInfo("Connected to " << peer);

        sockets_[peer] = std::shared_ptr<tcp_fragment_wrapper>(new tcp_fragment_wrapper(
            sock, boost::bind(&net_worker::on_recieve, this, _1, _2, peer), [](error_code const& err){tcp_error(err);}/*boost::bind(&net_worker::disconnect, this, _1)*/, &tcp_error));  
        
        on_all_connected();

    }

private:
    std::map< endpoint,unique_ptr<async_connector>>                                      cons_;
    std::map<network::endpoint, std::shared_ptr<tcp_fragment_wrapper> >               sockets_;
    std::vector<endpoint>                                                        visapp_peers_;  
   
    ses_helper                                                                     ses_helper_;
private:
    const  endpoint                                                                  mod_peer_;
    const  endpoint                                                                      peer_;

private:
    on_receive_f                                                      on_receive_;
    on_ses_receive_f                                                 on_ses_recv_;
    on_update_f                                                        on_update_;
    on_all_connected_f                                          on_all_connected_;
private:
    bool                                                                    done_;

private:
    double                                                                period_;

private:
    net::timer_connection                                            calc_timer_ ;
    net::ses_srv*                                                            ses_;
};


struct mod_app
{
    typedef boost::function<void(run_msg const& msg)>                   on_run_f;
    typedef boost::function<void(container_msg const& msg)>       on_container_f;

    mod_app(endpoint peer/*,  binary::bytes_cref bytes*/)
        : systems_    (get_systems(systems::SECOND_IMPL, boost::bind(&mod_app::send, this, _1)))
        , ctrl_sys_   (systems_->get_control_sys())
        , mod_sys_    (systems_->get_model_sys  ())
        , disp_       (boost::bind(&mod_app::inject_msg      , this, _1, _2))
        , init_       (false)
        , dc_         (false)
        , peers_ready_(0)

    {   

        disp_
            .add<setup_msg             >(boost::bind(&mod_app::on_setup      , this, _1))
            .add<state_msg             >(boost::bind(&mod_app::on_state      , this, _1))
            .add<vis_peers_msg         >(boost::bind(&mod_app::on_vis_peers  , this, _1))
            .add<ready_msg             >(boost::bind(&mod_app::on_ready      , this, _1))

            ;                                   

        w_.reset (new net_worker( peer 
            , boost::bind(&msg_dispatcher<uint32_t>::dispatch, &disp_, _1, _2, 0/*, id*/)
            , boost::bind(&mod_app::on_recv, this, _1, _2)
            , boost::bind(&mod_app::update, this, _1)
            , boost::bind(&mod_app::on_setup_deffered, this)
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
    
    void send (binary::bytes_cref bytes)
    {
        w_->send_clients(bytes);
    }
    
    void on_recv (void const* data, size_t size)
    {
        systems_->push_back(std::move(binary::make_bytes(data,size)));
    }

    void update(double time)
    {   
        time_measure_helper_t th("mod_app::update: ", [=](double t)->bool{return true; }); 

        systems_->update_messages();
        ctrl_sys_->update(time);
        systems_->update_messages();
        
        time_measure_helper_t th1("mod_app::update: ( mod_sys ): ", [=](double t)->bool{return true; }); 
        mod_sys_->update(time);
        deffered_create();
    }


    void on_setup(setup_msg const& msg)
    {
        w_->set_factor(0.0);
        
        setup_msg_ = std::move(msg);
    }

    void on_setup_deffered()
    {
        init_ = true;
        //send_props();
        create_objects(setup_msg_);
    }

    void send_props() 
    {
        std::string props =  "base_point\n{\n    lat 55.9724\n    lon 37.4131\n    height 0\n}\nchannel\n{\n    hfov 60\n    hdeflection 0\n    course 60\n    pixel_scale 1\n    camera_name \"\"\n    cylindric_geom_corr false\n}\nwindow\n{\n    ltcorner\n    {\n        x 0\n        y 0\n    }\n    size\n    {\n        x 500\n        y 500\n    }\n    fullscreen false\n}\n";
        binary::bytes_t bts =  std::move(wrap_msg(props_updated(props)));
        w_->send_session_clients(bts);
    }


    void on_state(state_msg const& msg)
    {
       w_->set_factor(msg.factor);
       w_->reset_time(msg.srv_time / 1000.0f);
    }

    void on_ready(ready_msg const& msg)
    {
        peers_ready_++;
        LogInfo(" on_ready: " << peers_ready_ << " vis_peers_.size() = " << vis_peers_.size() );
        if (peers_ready_ == vis_peers_.size() )
           all_ready() ;
    }


    void deffered_create()
    {
        if(init_ /*&& !dc_*/)
        {
 #if 0
         for(auto it = creation_deque_.begin(); it != creation_deque_.end(); ++it )
             on_create(*it);
#endif
         
          creation_deque_.clear();
          
          dc_ = true; 
        }
    }

    void all_ready() 
    {
        binary::bytes_t bts =  std::move(wrap_msg(ready_msg(0)));
        w_->send_proxy(&bts[0], bts.size());
    }


    void on_vis_peers(vis_peers_msg const& msg)
    {
        vis_peers_ = msg.eps;
        w_->vis_connect(msg.eps);
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

        systems_->create_auto_objects();

        if(auto fp = fn_reg::function<void(const std::string&)>("create_objects"))
            fp(airport);

        reg_obj_ = objects_reg::control_ptr(find_object<object_info_ptr>(dynamic_cast<kernel::object_collection*>(ctrl_sys_.get()),"aircraft_reg")) ;   

        if (reg_obj_)
        {
            void (net_worker::*send_)       (binary::bytes_cref bytes)              = &net_worker::send_proxy;

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
            void (net_worker::*send_)       (binary::bytes_cref bytes)              = &net_worker::send_proxy;

            reg_obj_->set_sender(boost::bind(send_, w_.get(), _1 ));
        }

        binary::bytes_t bts =  std::move(network::wrap_msg(create_session(std::string("session_name"), binary::wrap(dict),0.0)));
        w_->send_session_clients(bts);
        
        mod_sys_->load_exercise(dict);

    }

private:
    systems_ptr                                                 systems_;
    kernel::system_ptr                                          mod_sys_;
    kernel::system_ptr                                         ctrl_sys_;

private:
    msg_dispatcher<uint32_t>                                       disp_;
    std::vector<endpoint>                                     vis_peers_;
    uint16_t                                                peers_ready_; 
    std::deque<create_msg>                               creation_deque_;
    bool                                                           init_;
    bool                                                             dc_;
private:
    on_container_f                                              on_cont_;
    setup_msg                                                 setup_msg_;
private:
    boost::scoped_ptr<net_worker>                                     w_;

private:
    objects_reg::control_ptr                                    reg_obj_;

};


}

namespace {
    inline void timer_res()
    {

#ifdef _WIN32
        TIMECAPS  timecaps;  // требуется для функции timeGetDevCaps

        // получить max & min of системного таймера
        if ( timeGetDevCaps( &timecaps, sizeof( TIMECAPS ) ) == TIMERR_NOERROR )
        {
            // получить оптимальное разрешение
            UINT wTimerRes = std::max( timecaps.wPeriodMin, UINT(1) );

            // установить минимальное разрешение для нашего таймера
            if( timeBeginPeriod( wTimerRes ) != TIMERR_NOERROR )
            {
                // здесь происходит ошибка
            }
        }
#endif

    }

    inline void hide_console()
    {
#ifdef _WIN32
        ShowWindow( GetConsoleWindow(), SW_HIDE );
#endif

    }



}


int main_modapp( int argc, char** argv )
{
    
    logger::need_to_log(/*true*/);
    logging::add_console_writer();

    timer_res();
    //hide_console();

    asio2asio_initializer   asi;

    try
    {

        endpoint peer(cfg().network.local_address);
        mod_app ma(peer);

	    asi.run_main();
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

AUTO_REG(main_modapp)
