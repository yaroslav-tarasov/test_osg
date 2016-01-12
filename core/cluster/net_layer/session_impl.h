#pragma once 

#include "ses_messages.h"

namespace net
{
    using namespace  net_layer;

    // deferred call is used ONLY by terminal timer
    // it's invoked to do special clean-up before session stops 
    typedef boost::function<void()>                         timer_callback_f;
    typedef boost::function<void(timer_callback_f const&)>  deferred_call_f;


// doesn't need to link to net_layer
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


struct  session_impl
    : ses_srv
{
    typedef boost::function<void(binary::bytes_cref, bool /* sure */)>  send_f;
    typedef boost::function<double()>                                   time_func_f;

    //! конструктор; вызываетс€ в net_srv.ses_created() по нажатию "v"
    session_impl(/*net_srv& srv, send_f const& send,*/ time_func_f const& srv_time, /*bool control,*/ double initial_time/*, deferred_call_f const& dfrd_cal*/);

    ~session_impl();

    void on_recv(binary::bytes_cref bytes, bool loopback);
    bool control() const;

private:
    DECL_LOGGER("net_layer.session_controller");

    // ses_srv
public:
    void                send        (binary::bytes_cref bytes, bool sure)   override;
    double              time        () const                                override;
    void                set_factor  (double factor)                         override;
    void                reset_time  (double new_time)                       override;

    timer_connection    create_timer(double period_sec, on_timer_f const&, bool adjust_time_factor, bool terminal) override;

    void                session_data_loaded() override;
    bool                local       () const;
#if 0 
    net_srv&            net_server  () const;
#endif

    // фантазии
    void                stop();

    // public interface for net_server usage
public:
    void update_time(double old_time);

public:
    DECLARE_EVENT(session_stopped, ());

private:
    void on_new_time_response   (ses_msg::new_time_response const& msg);
    void on_data                (ses_msg::data_msg const& msg, bool loopback);
    void on_session_loaded      ();

#if 0
private:
    net_srv& net_srv_;
#endif

private:
    bool                         control_;

#if 0
    network::msg_dispatcher<bool>   disp_; // bool - loopback
#endif

    send_f                          send_;
    time_func_f                     srv_time_;

private:
    time_control         time_;

private:
    deferred_call_f dfrd_call_;
};

typedef boost::shared_ptr<session_impl> session_impl_ptr;


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


}