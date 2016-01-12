#include "stdafx.h"
#include "net_layer/net_worker.h"
#include "session_impl.h"

#include "async_services/async_timer.h"
#include "common/locks.h"

namespace 
{
    using namespace net;
    using namespace net_layer;
    
    struct session_timer 
        : net::details::timer_holder
    {

        //static connection create (details::session* ses,double period_sec, on_timer_f const& on_timer, bool adjust_time_factor, bool terminal = false)
        //{
        //    return connection(
        //        new session_timer(
        //        ses, 
        //        on_timer, 
        //        period_sec, 
        //        1/*time_.get_factor()*/, 
        //        adjust_time_factor, 
        //        /*terminal ? dfrd_call_ :*/ deferred_call_f()));
        //}

        session_timer( session_impl* session, ses_srv::on_timer_f  const& on_timer, double period, double factor, bool adjust_time_factor, deferred_call_f const& def_call)
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
        session_impl*           session_;
        ses_srv::on_timer_f    on_timer_;
        double                   period_;
        double                   factor_;
        double              last_time_point_;
        bool                adjust_time_factor_;
        async_timer         timer_;

    private:
        scoped_connection   changing_time_factor_;
        scoped_connection   stopped_session_;

    private:
        deferred_call_f     deferred_call_;


    };

}


namespace net
{
    using namespace ses_msg;

    session_impl::session_impl ( time_func_f const& srv_time, double initial_time )
        : time_(initial_time)
        , srv_time_ (srv_time)
    {
        time_.set_factor(0.0);
    }


    // native
    double session_impl::time() const
    {
        return time_.time();
    }

    // фантазии
    void session_impl::stop()
    {
        time_reset_signal_(/*ses_time*/time(), 0);
    }


    // native not realized (not used?)
    void session_impl::send(binary::bytes_cref bytes, bool sure)
    {
        //Assert(send_);
        //send_(*network::wrap(data_msg(bytes)), sure);
    }

    // modified
    void session_impl::reset_time(double new_time)
    {
        // сообщение в сеть с новым временем и текущим фактором

        time_.set_factor(time_.get_factor(), new_time);
        time_reset_signal_(new_time, time_.get_factor());
    }

    // modified
    void session_impl::set_factor(double factor)
    {
        // сообщение в сеть с новым фактором только, время не устанавливается

        time_.set_factor(factor);
    }

    // modified   
    bool session_impl::local() const
    {
        return true/*control_*/;
    }

    // native
    void session_impl::session_data_loaded()
    {
        session_loaded_signal_();
    }

    void session_impl::on_new_time_response(new_time_response const& msg)
    {
        double ses_time = std::max(0., msg.ses_time + (srv_time_() - msg.srv_time) * msg.factor);

        time_.set_factor(msg.factor, ses_time);
        time_reset_signal_(ses_time, msg.factor);
    }

    //net_layer::net_srv&            net_server  () const 
    //{
    //       return net_layer::net_srv();
    //}

    session_impl::~session_impl()
    {
        session_stopped_signal_();
    }

    timer_connection session_impl::create_timer(double period_sec, on_timer_f const& on_timer, bool adjust_time_factor, bool terminal)
    {
        return timer_connection(
            new session_timer(
            this, 
            on_timer, 
            period_sec, 
            time_.get_factor(), 
            adjust_time_factor, 
            terminal ? dfrd_call_ : deferred_call_f()));
    }
 
}