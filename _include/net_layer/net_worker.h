#pragma once 


namespace net
{
    // from net_server
    namespace details
    {
        struct timer_holder { virtual ~timer_holder(){} };
    } // details

    typedef boost::shared_ptr<details::timer_holder> timer_connection;

    struct  ses_srv
    {
        typedef boost::function<void (double /*time*/)> on_timer_f;

        //! сигнал Приняты данные
        DECLARE_EVENT(receive       , (binary::bytes_cref, bool /*loopback*/));
        //! сигнал
        DECLARE_EVENT(time_reset    , (double /*time*/, double /*factor*/));
        //! сигнал Сессия загружена
        DECLARE_EVENT(session_loaded, ());

        virtual void                send        (binary::bytes_cref bytes, bool sure)       = 0;
        virtual double              time        () const                                    = 0;

        // фантазии
        virtual void                stop        ()                                          = 0;

        virtual void                set_factor  (double factor)                             = 0;
        virtual void                reset_time  (double new_time)                           = 0;

        // 'adjust time factor' means timer is adjusted to session time 
        // 'terminal' means that this timer function invocation ALWAYS(!) stops session
        virtual timer_connection    create_timer(double period_sec, on_timer_f const&, bool adjust_time_factor = true, bool terminal = false)  = 0;

        virtual void                session_data_loaded()                                   = 0;

        virtual bool                local       () const                                    = 0;
#if 0
        virtual net_srv&            net_server  () const                                    = 0;
#endif

        virtual ~ses_srv() {}
    };

}


namespace net_layer
{
    net::ses_srv*    create_session  (binary::bytes_cref data, bool local, double initial_time = 0.);
}