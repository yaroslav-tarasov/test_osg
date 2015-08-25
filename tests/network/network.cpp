// network.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "async_services/async_services.h"
#include "network/msg_dispatcher.h"
#include "logger/logger.hpp"
#include "common/test_msgs.h"

using network::endpoint;
using network::async_acceptor;
using network::async_connector;
using network::tcp_socket;
using network::error_code;

using network::msg_dispatcher;

typedef boost::system::error_code       error_code_t;

using namespace net_layer::test_msg;
using network::tcp_fragment_wrapper;

namespace
{
    boost::asio::io_service* __main_srvc__ = 0;

    void tcp_error(error_code const& err)
    {
        LogError("Error happened: " << err);
        __main_srvc__->stop();
    }
}

struct client
{

    DECL_LOGGER("visa_control");

    client(endpoint peer)
        : con_(peer, boost::bind(&client::on_connected, this, _1, _2), tcp_error, tcp_error)
        , timer_  (boost::bind(&client::update, this))
        , ac_counter_(0)
    {
        LogInfo("Connecting to " << peer);

        disp_
            .add<ready_msg                 >(boost::bind(&client::on_remote_ready      , this, _1))
            ;
    }

    // from struct tcp_connection
    void send(void const* data, uint32_t size)
    {
        error_code_t ec;
        srv_->send(&size, sizeof(uint32_t));
        if (ec)
        {
            LogError("TCP send error: " << ec.message());
            return;
        }

        srv_->send(data, size);
        if (ec)
        {
            LogError("TCP send error: " << ec.message());
            return;
        }

    }

private:
    void on_connected(network::tcp::socket& sock, network::endpoint const& peer)
    {
        LogInfo("Connected to " << peer);
        // srv_ = in_place(boost::ref(sock), boost::bind(&msg_dispatcher<network::endpoint>::dispatch, disp_, _1, _2, peer), &tcp_error, &tcp_error);
        
        srv_ = std::shared_ptr<tcp_fragment_wrapper>(new tcp_fragment_wrapper(
            sock, boost::bind(&msg_dispatcher<endpoint>::dispatch, &disp_, _1, _2, peer), &tcp_error, &tcp_error));  

        binary::bytes_t bts =  std::move(wrap_msg(setup(1111,2222)));
        send(&bts[0], bts.size());

    }

    void on_remote_ready(uint16_t value)
    {
        start_send();
    }
    
    inline void start_send()
    {
        update();
    }

    void update()
    {   
        binary::bytes_t bts =  std::move(wrap_msg(run(1111,2222)));
        send(&bts[0], bts.size());
        
        LogInfo("update() send run " );

        if(ac_counter_++==40)
        {
            binary::bytes_t bts =  std::move(wrap_msg(create(0,0,90)));
            send(&bts[0], bts.size());

            LogInfo("update() send create " );
        }

        if(ac_counter_==45)
        {
            binary::bytes_t bts =  std::move(wrap_msg(create(0,0.005,0)));
            send(&bts[0], bts.size());

            LogInfo("update() send create " );
        }

        timer_.wait(boost::posix_time::microseconds(int64_t(1e6 * /*period_*/1)));
    }

private:
    async_timer             timer_; 

private:
    async_connector         con_;
    // optional<tcp_socket>    srv_;
    std::shared_ptr<tcp_fragment_wrapper>  srv_;

    msg_dispatcher<network::endpoint>                                    disp_;

private:
    uint32_t                  ac_counter_;
};


int _tmain(int argc, _TCHAR* argv[])
{
    async_services_initializer asi(false);
    
    logging::add_console_writer();

    __main_srvc__ = &(asi.get_service());
    
    try
    {
        endpoint peer(std::string("127.0.0.1:30000"));
        client c(peer);              
        __main_srvc__->run();
    }
    catch(verify_error const&)
    {
        return -1;
    }
    catch(std::exception const& err)
    {
        LogError("Exception caught: " << err.what());
        return -1;
    }

	return 0;
}

