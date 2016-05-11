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
#include "common/time_counter.h"
#include "utils/high_res_timer.h"
#include "tests/systems/factory_systems.h"
#include "objects/registrator.h"

#include "net_layer/net_worker.h"

#include "reflection/proc/prop_tree.h"


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
     typedef boost::function<void(const void* data, size_t size, endpoint peer)>   on_receive_f;
     typedef boost::function<void(const void* data, size_t size)>                  on_ses_receive_f;

     typedef boost::function<void(double time)>                                    on_update_f;     

     struct ses_helper
     {
         ses_helper (net_worker* nw)
             : nw_ (nw)
         {}

         void on_accepted(network::tcp::socket& sock, endpoint const& peer)
         {
             const uint32_t id = peer.port;
             socket_ = std::shared_ptr<tcp_fragment_wrapper>(new tcp_fragment_wrapper( sock 
                 , boost::bind(&ses_helper::on_recieve, this, _1,_2, peer)
                 , boost::bind(&net_worker::on_disconnected, nw_, _1, id)
                 , boost::bind(&net_worker::on_error, nw_, _1, id)
                 ));        

             LogInfo("Client " << peer << " accepted");
         }

         void on_recieve(const void* data, size_t size, endpoint const& peer)
         {
             __main_srvc__->post(boost::bind(&ses_helper::do_recieve, this, binary::make_bytes_ptr(data, size) ));
         }

         void do_recieve(binary::bytes_ptr data )
         {
             if (nw_->on_ses_recv_)
                 nw_->on_ses_recv_(binary::raw_ptr(*data), binary::size(*data));
         }

         std::shared_ptr<tcp_fragment_wrapper>   socket_;
         net_worker*                             nw_;
     };

     net_worker(const  endpoint &  peer, const endpoint& mod_peer, on_receive_f on_recv ,on_ses_receive_f on_ses_recv , on_update_f on_update)
         : period_     (cfg().model_params.msys_step)
         , ses_        (net_layer::create_session(binary::bytes_t(), true))
         , mod_peer_   (mod_peer)
         , peer_       (peer)
         , on_receive_ (on_recv)
         , on_ses_recv_ (on_ses_recv)
         , on_update_  (on_update)
         , done_       (false)
         , ses_helper_ (this)
     {
          worker_thread_ = boost::thread(&net_worker::run, this);
     }
     
     ~net_worker()
     {
         worker_thread_.join();
         delete  ses_;
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

         ses_acc_.reset(new  async_acceptor (peer_, boost::bind(&ses_helper::on_accepted, ses_helper_, _1, endpoint(std::string("0.0.0.0:45003")) ), tcp_error));
         mod_acc_.reset(new  async_acceptor (mod_peer_, boost::bind(&net_worker::on_accepted, this, _1, endpoint(std::string("0.0.0.0:45002")) ), tcp_error));

         worker_service_ = &(asi.get_service());
         
         boost::asio::io_service::work skwark(asi.get_service());

         calc_timer_   = ses_->create_timer ( period_, boost::bind(&net_worker::on_timer, this ,_1) , 1, false);

		 boost::system::error_code ec;
         size_t ret = worker_service_->run(ec);


         ses_acc_.reset();
         mod_acc_.reset();
         calc_timer_.reset();
         sockets_.clear();

         __main_srvc__->post(boost::bind(&boost::asio::io_service::stop, __main_srvc__));
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
         const uint32_t id = peer.port;
         if(sockets_.size()==0)
         {
             sockets_[id] = std::shared_ptr<tcp_fragment_wrapper>(new tcp_fragment_wrapper( sock
                 , boost::bind(&net_worker::on_recieve, this, _1,_2, peer)
                 , boost::bind(&net_worker::on_disconnected, this, _1, id)
                 , boost::bind(&net_worker::on_error, this, _1, id)
                 ));        
             LogInfo("Client " << peer << " accepted");
         }
         else
         {
             LogInfo("Client " << peer << " rejected");
         }
     }

     void on_recieve(const void* data, size_t size, endpoint const& peer)
     {
         __main_srvc__->post(boost::bind(&net_worker::do_receive, this, binary::make_bytes_ptr(data, size) , peer));
     }

     void do_receive(binary::bytes_ptr data , endpoint const& peer) 
     {
         if (on_receive_)
             on_receive_(binary::raw_ptr(*data), binary::size(*data) , peer);
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
         worker_service_->post(boost::bind(&boost::asio::io_service::stop, worker_service_));
     }

     void on_error(boost::system::error_code const& ec, uint32_t sock_id)
     {
         LogError("TCP error: " << ec.message());
         sockets_.erase(sock_id);
         // peers_.erase(std::find_if(peers_.begin(), peers_.end(), [sock_id](std::pair<id_type, uint32_t> p) { return p.second == sock_id; }));
     }

private:
    boost::scoped_ptr<async_acceptor>                           ses_acc_       ;
    boost::scoped_ptr<async_acceptor>                           mod_acc_   ;
    std::map<uint32_t, std::shared_ptr<tcp_fragment_wrapper> >  sockets_;
    
    ses_helper                                                  ses_helper_;
private:
    boost::thread                                               worker_thread_;
    boost::asio::io_service*                                    worker_service_;
    std::shared_ptr<boost::asio::io_service::work>              work_;
    const  endpoint                                             mod_peer_;
    const  endpoint                                             peer_;

private:
    on_receive_f                                                on_receive_;
    on_ses_receive_f                                            on_ses_recv_;
    on_update_f                                                 on_update_;


private:
    bool                                                        done_;

private:
    double                                                      period_;

private:
    net::timer_connection                                       calc_timer_ ;
    net::ses_srv*                                               ses_;
};




struct global_timer : boost::noncopyable
{
    inline void set_time(double time)
    {
         internal_time(time);
         force_log fl;       
         LOG_ODS_MSG( " void set_time(double time)  = " << time << "\n");
    }

    inline double get_time()
    {
        return internal_time();
    }

    inline void set_factor(double factor)
    {
        internal_factor(factor);
        force_log fl;       
        LOG_ODS_MSG( " void set_factor(double factor)  = " << factor << "\n");
    }

private:
    
    double internal_time(double time = -1.0)
    {
        static double                 time_         = 0.0;
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

    visapp_impl(kernel::vis_sys_props const& props/*, binary::bytes_cref bytes*/, kernel::msg_service& msg_srv)
        : osg_vis_  (av::CreateVisual())
        , msg_srv_  (msg_srv)
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
    
    void update_property(kernel::vis_sys_props const& props)
    {
        kernel::visual_system_props_ptr(vis_sys_.get_sys())->update_props(props);
    }

private:

    void on_render(double time)
    {   
        vis_sys_.update(time);
        osg_vis_->Render(time);
    }

private:

    kernel::visual_system_ptr create_vis(kernel::vis_sys_props const& props, av::IVisual* vis/*, binary::bytes_cref bytes*/)
    {
        using namespace binary;
        using namespace kernel;

        return create_visual_system(msg_srv_, vis, props);
    }

private:
	av::IVisualPtr                                              osg_vis_;
    kernel::msg_service&                                        msg_srv_;
    updater                                                     vis_sys_;


};

struct visapp
{
    visapp( const  endpoint &  peer, const endpoint& mod_peer)
        : done_          (false)
        , thread_        (new boost::thread(boost::bind(&visapp::run, this)))
        , msg_service_   (boost::bind(&visapp::push_back, this, _1))
		, need_to_update_(false)
    {
        
        props_.base_point = ::get_base();
        props_.channel.camera_name = "camera 0";

        disp_
            .add<setup                 >(boost::bind(&visapp::on_setup      , this, _1))
            .add<state                 >(boost::bind(&visapp::on_state      , this, _1))
            .add<props_updated         >(boost::bind(&visapp::on_props_updated   , this, _1))
            ;

        w_.reset (new net_worker( peer, mod_peer 
            , boost::bind(&visapp::on_recv,this, _1, _2, _3)
            , boost::bind(&msg_dispatcher<uint32_t>::dispatch, &disp_, _1, _2, 0)
            , boost::bind(&visapp::update, this, _1)
            ));

    }


    ~visapp()
    {

        done();
        thread_->join();
        w_.reset();
    }
    
    void on_recv(void const* data, size_t size, const endpoint& peer )
    {
        queue_vis_.emplace_back(std::move(binary::make_bytes(data,size)));
    }
    
    void update_messages()
    {
        while(queue_vis_.size()>0)
        {
            if(queue_vis_.front().size()>0)
                msg_service_.on_remote_recv(queue_vis_.front(),true);

            queue_vis_.pop_front();
        }
    }
	
	void update_properties(visapp_impl& vi)
	{
		if (need_to_update_)
		{
			vi.update_property(props_);
			need_to_update_ = false;
		}
	}

    void push_back (binary::bytes_cref bytes)
    {
        w_->send(bytes);
    }

    void update(double time)
    {    
        gt_.set_time(time);
        //force_log fl;       
        LOG_ODS_MSG( " void visapp::update(double time) " << time << "\n");
    }
 
    void on_setup(setup const& msg)
    {
        w_->set_factor(0.0);
        gt_.set_factor(0.0);
    }

    void on_state(state const& msg)
    {
        gt_.set_factor(msg.factor);
        w_->set_factor(msg.factor);
        w_->reset_time(msg.srv_time / 1000.0f);
        gt_.set_time(/*time*/msg.srv_time / 1000.0f);
        force_log fl;       
        LOG_ODS_MSG( " void visapp::on_state(double time) " << msg.srv_time / 1000.0f << "\n");
    }



    void run()
    {   
        visapp_impl  va( props_, msg_service_);
        end_of_load_  = boost::bind(&visapp_impl::end_this,&va);
        end_of_load_();
        while (!va.done() && !done_)
        {
          update_messages();
		  update_properties(va);
          va.on_render(gt_.get_time());
          //force_log fl;       
          LOG_ODS_MSG( " void run() time  = " << gt_.get_time() << "\n");
        }
    }
    
    void done()
    {
        done_ = true;
    }
    
   
    void on_props_updated(props_updated const& msg)
    {
        kernel::vis_sys_props props;
        std::stringstream is(msg.properties);
        prop_tree::read_from(is, props);
        
        FIXME(Чего-то со свойствами надо делать);
        props_ = props;
        props_.base_point = ::get_base();
		need_to_update_ = true;
    }

private:
    kernel::vis_sys_props               props_;
	bool								need_to_update_;
private:
    bool                                 done_;
    std::unique_ptr<boost::thread>     thread_;
    boost::function<void()>       end_of_load_;

private:
    msg_dispatcher<uint32_t>             disp_;
    kernel::msg_service           msg_service_;

private:
    std::deque<binary::bytes_t>     queue_vis_;

private:
    global_timer                           gt_;

private:
    boost::scoped_ptr<net_worker>           w_;

};


}

int main_visapp( int argc, char** argv )
{
    
    logger::need_to_log(/*true*/);
    logging::add_console_writer();

    boost::asio::io_service  service_;
    typedef boost::shared_ptr<boost::asio::io_service::work> work_ptr;
    work_ptr dummy_work(new boost::asio::io_service::work(service_));
    

    __main_srvc__ = &service_;

    try
    {

        endpoint proxy_peer(std::string("0.0.0.0:45003"));
        endpoint mod_peer  (std::string("0.0.0.0:45002"));

        visapp  va(proxy_peer, mod_peer);

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

AUTO_REG(main_visapp)
