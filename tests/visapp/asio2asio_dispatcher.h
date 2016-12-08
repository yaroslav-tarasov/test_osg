#pragma once

namespace network
{

    namespace asio2asio
    {

        struct dispatcher
            : boost::noncopyable
        {
            typedef  boost::function<void()> start_f;
            typedef  boost::function<void()> stop_f;
            typedef  boost::function<void()> fwd_f;

            dispatcher()
                : io_   (nullptr)
                //, start_(start)
                //, stop_ (stop)
            {
               thread_ = in_place(bind(&dispatcher::run, this));
            }

            ~dispatcher()
            {
                if (thread_ && io_)
                {
                    // to allow io_service to process all pending events
                    io_->post(bind(&boost::asio::io_service::stop, boost::ref(io_)));

                    thread_->join();
                }
                else 
                    io_->stop();

                main_.stop();

            }

            // all parameters should be given only by value
            void call_asio(fwd_f const& func)
            {
                if (thread_ && io_)
                    io_->post(func);
                else
                    func();
            }

            // all parameters should be given only by value
            void call_main(fwd_f const& func)
            {
                if (thread_)
                    main_.post(func);
                else
                    func();
            }

            boost::asio::io_service& get_service()
            {
                return main_;
            }

        private:
            void run()
            {
                async_services_initializer asi(false);
                
                if(start_)
                    start_();

                boost::system::error_code ec;
                io_ = &(asi.get_service());
                boost::asio::io_service::work w(asi.get_service());
                size_t ret = io_->run(ec);

                if(stop_)
                    stop_();
            }

        private:
            boost::asio::io_service* io_;
            boost::asio::io_service  main_;
            optional<boost::thread>  thread_;
            start_f                  start_;
            stop_f                   stop_;
        };




    //////////////////////////////
    /// dispatcher

    inline optional<asio2asio::dispatcher>& raw_disp()
    {
        static optional<asio2asio::dispatcher> d;
        return d;
    }


    inline asio2asio::dispatcher& disp()
    {
        return *raw_disp();
    }

    }

}