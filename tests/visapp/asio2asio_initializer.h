#pragma once

struct  asio2asio_initializer
    : boost::noncopyable
{
    typedef  boost::function<void()> start_f;
    typedef  boost::function<void()> stop_f;

    asio2asio_initializer(start_f, stop_f);
    ~asio2asio_initializer();

    static boost::asio::io_service& get_service();                                                 
}; 
