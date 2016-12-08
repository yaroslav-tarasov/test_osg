#pragma once

struct  asio2asio_initializer
    : boost::noncopyable
{
    asio2asio_initializer();
    ~asio2asio_initializer();

    static void run_main();
    static boost::asio::io_service& get_service();                                                 
}; 
