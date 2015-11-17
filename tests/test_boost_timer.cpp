//
// prioritised_handlers.cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "stdafx.h"
#include <boost/asio.hpp>
#include <boost/function.hpp>
#include <iostream>
#include <queue>

#include "async_services/async_services.h"
#include "common/locks.h"

using boost::asio::ip::tcp;

namespace {
    
    void sleep( int msec )
    {
        boost::xtime xt;
        boost::xtime_get(&xt, boost::TIME_UTC_/*boost::TIME_UTC*/);
        xt.sec += msec/1000;
        xt.nsec += msec * 1000000;

        boost::thread::sleep(xt);
    }

class handler_priority_queue
{
public:
    void add(int priority, boost::function<void()> function)
    {
        handlers_.push(queued_handler(priority, function));
    }

    void execute_all()
    {
        while (!handlers_.empty())
        {
            queued_handler handler = handlers_.top();
            handler.execute();
            handlers_.pop();
        }
    }

    // A generic wrapper class for handlers to allow the invocation to be hooked.
    template <typename Handler>
    class wrapped_handler
    {
    public:
        wrapped_handler(handler_priority_queue& q, int p, Handler h)
            : queue_(q), priority_(p), handler_(h)
        {
        }

        void operator()()
        {
            handler_();
        }

        template <typename Arg1>
        void operator()(Arg1 arg1)
        {
            handler_(arg1);
        }

        template <typename Arg1, typename Arg2>
        void operator()(Arg1 arg1, Arg2 arg2)
        {
            handler_(arg1, arg2);
        }
        
        template <typename Arg1, typename Arg2, typename Arg3 >
        void operator()(Arg1 arg1, Arg2 arg2, Arg3 arg3)
        {
            handler_(arg1, arg2, arg3);
        }

        //private:
        handler_priority_queue& queue_;
        int priority_;
        Handler handler_;
    };

    template <typename Handler>
    wrapped_handler<Handler> wrap(int priority, Handler handler)
    {
        return wrapped_handler<Handler>(*this, priority, handler);
    }

private:
    class queued_handler
    {
    public:
        queued_handler(int p, boost::function<void()> f)
            : priority_(p), function_(f)
        {
        }

        void execute()
        {
            function_();
        }

        friend bool operator<(const queued_handler& a,
            const queued_handler& b)
        {
            return a.priority_ < b.priority_;
        }

    private:
        int priority_;
        boost::function<void()> function_;
    };

    std::priority_queue<queued_handler> handlers_;
};

// Custom invocation hook for wrapped handlers.
template <typename Function, typename Handler>
void asio_handler_invoke(Function f,
    handler_priority_queue::wrapped_handler<Handler>* h)
{
    h->queue_.add(h->priority_, f);
}

//----------------------------------------------------------------------

void high_priority_handler(const boost::system::error_code& /*ec*/)
{
    std::cout << "High priority handler\n";
}

struct mhandler
{
    mhandler(boost::asio::io_service& io_service, handler_priority_queue& pri_queue, int priority, size_t interval_ms)
        : io_service_(io_service)
        , timer_     (io_service)
        , pri_queue_ (pri_queue)
        , priority_  (priority)
        , interval_  (interval_ms)
    {
        timer_.expires_at(boost::posix_time::neg_infin);
        timer_.async_wait(pri_queue.wrap(priority_, boost::bind(&mhandler::middle_priority_handler,this,_1)));
    }

    void middle_priority_handler(const boost::system::error_code& /*ec*/)
    {
        timer_.expires_from_now(/*dtime_*/boost::posix_time::microseconds(int64_t(1e3 * interval_)));
        timer_.async_wait(pri_queue_.wrap(priority_, boost::bind(&mhandler::middle_priority_handler,this,_1)));
        std::cout << "Middle priority handler" << priority_  << "\n";
    }

    boost::asio::deadline_timer timer_;
    boost::asio::io_service&    io_service_;
    handler_priority_queue&     pri_queue_;
    int                         priority_;
    size_t                      interval_;

}; 



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
    struct timer_holder { 
        virtual ~timer_holder(){} 
        virtual void set_factor(double factor) = 0;
        virtual double get_factor() = 0;
    };

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
            time_reset_signal_(/*ses_time*/time(), factor);
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

boost::asio::io_service* __main_srvc__ = 0;
handler_priority_queue*  __priority_queue__ = 0;
const float              init_rp            = 1/60.f;
float                    __render_period__  = init_rp;
float                    __new_render_period__ = init_rp;
struct net_worker
{
    typedef boost::function<void(const void* data, size_t size)>   on_receive_f;
    typedef boost::function<void(double time)>                     on_update_f;     



    net_worker( on_update_f on_update, on_update_f on_render)
        : period_     (/*cfg().model_params.msys_step*/0.05)
        , ses_        (new details::session(0))
        , on_update_  (on_update)
        , on_render_  (on_render)
        , done_       (false)
    {
        _workerThread = boost::thread(&net_worker::run, this);
    }

    ~net_worker()
    {
        delete  ses_;
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

private:
    void run()
    {
        async_services_initializer asi(false);

        _workerService = &(asi.get_service());

        boost::asio::io_service::work skwark(asi.get_service());

#if 0
        render_timer_ = session_timer::create (ses_, __render_period__,  [&](double time)
        {
            __main_srvc__->post(__priority_queue__->wrap(100, boost::bind(on_render_,time))/*boost::bind(on_render_,time)*/);
           
            std::cout << "on_render handler:  __render_period__  - __new_render_period__: " <<  __render_period__  - __new_render_period__ << " \n";
            
            render_timer_->set_factor( /*render_timer_->get_factor()*//* - 1  +*/ __render_period__ / __new_render_period__  );
            
        } , 1, false);
#endif

        calc_timer_   = session_timer::create (ses_, 0.05f, boost::bind(&net_worker::on_timer, this ,_1) , 1, true);


        boost::system::error_code ec;
        size_t ret = _workerService->run(ec);

    }

    uint32_t next_id()
    {
        static uint32_t id = 0;
        return id++;
    }


private:

    void on_timer(double time)
    {
        if(done_)
        {
            ses_->stop();
            return;
        }

        __main_srvc__->post(__priority_queue__->wrap(100, boost::bind(on_update_,time)));
        //on_update_(time);
    }


private:
    boost::thread                                               _workerThread;
    boost::asio::io_service*                                    _workerService;
    std::shared_ptr<boost::asio::io_service::work>              _work;

private:
    on_receive_f                                                on_receive_;
    on_update_f                                                 on_update_;
    on_update_f                                                 on_render_;

private:
    bool                                                          done_;

private:
    double                                                       period_;

private:
    session_timer::connection                                 render_timer_;
    session_timer::connection                                 calc_timer_ ;
    session_timer::connection                                 ctrl_timer_;
    details::session*                                         ses_;
};

struct testapp
{

    testapp(int argc, char** argv)

    {   


        w_.reset (new net_worker( 
              boost::bind(&testapp::update, this, _1)
            , boost::bind(&testapp::on_render, this, _1)
            ));


    }

    ~testapp()
    {
        w_.reset();
    }

    void render()
    {
         on_render(0);
    }

private:

    void on_render(double time)
    {   
        high_res_timer                _hr_timer;
        
        sleep(40);
        //LogInfo( "on_render(double time)" << _hr_timer.get_delta());
        std::cout << "on_render handler:  " << time << " delta: " << _hr_timer.get_delta() << " \n";
        __new_render_period__   = _hr_timer_render.get_delta();
    }

    void update(double time)
    {   
        double sim_time = 1.0;

        if(sim_time  < 0)
        {
            w_->done();
            __main_srvc__->stop();
            return;
        }
        else
        {
            
            std::cout << "update handler  " << _hr_timer_update.get_delta() <<  " \n";
        }

    }


    uint32_t next_id()
    {
        static uint32_t id = 0;
        return id++;
    }

private:
    boost::scoped_ptr<net_worker>                                     w_;
    high_res_timer                _hr_timer_render;
    high_res_timer                _hr_timer_update;
};

void low_priority_handler()
{
    std::cout << "Low priority handler\n";
}

void empty_handler()
{
}


}

int main_asio_test(int argc, char** argv)
{
    boost::asio::io_service io_service;
    handler_priority_queue pri_queue;

    __main_srvc__ = &io_service;
    __priority_queue__ = &pri_queue;
    typedef boost::shared_ptr<boost::asio::io_service::work> work_ptr;
    work_ptr dummy_work(new boost::asio::io_service::work(io_service));



    // Post a completion handler to be run immediately.
    io_service.post(pri_queue.wrap(0, low_priority_handler));

    // Start an asynchronous accept that will complete immediately.
    tcp::endpoint endpoint(boost::asio::ip::address_v4::loopback(), 0);
    tcp::acceptor acceptor(io_service, endpoint);
    tcp::socket server_socket(io_service);
    acceptor.async_accept(server_socket,
        pri_queue.wrap(100, high_priority_handler));
    tcp::socket client_socket(io_service);
    client_socket.connect(acceptor.local_endpoint());

    // Set a deadline timer to expire immediately.
    //boost::asio::deadline_timer timer(io_service);
    //timer.expires_at(boost::posix_time::neg_infin);
    //timer.async_wait(pri_queue.wrap(43, boost::bind(&middle_priority_handler,timer,pri_queue,_3)));
    
    //mhandler model_update(io_service,pri_queue,20,1000);

    //mhandler vis_update(io_service,pri_queue,43,100);

    try
    {

        testapp s(argc, argv);

        boost::system::error_code ec;
        while (io_service.run_one())
        {
            // The custom invocation hook adds the handlers to the priority queue
            // rather than executing them from within the poll_one() call.
            while (io_service.poll_one())
                ;
            
            pri_queue.execute_all();

            //s.render();

            io_service.post(pri_queue.wrap(100, boost::bind(&testapp::render,&s)));
        }

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

AUTO_REG(main_asio_test)