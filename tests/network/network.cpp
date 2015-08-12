// network.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "async_services/async_services.h"
#include "logger/logger.hpp"
#include "common/test_msgs.h"

using network::endpoint;
using network::async_acceptor;
using network::async_connector;
using network::tcp_socket;
using network::error_code;

typedef boost::system::error_code       error_code_t;

using namespace net_layer::test_msg;

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

    DECL_LOGGER("visa_client");

    client(endpoint peer)
        : con_(peer, boost::bind(&client::on_connected, this, _1, _2), tcp_error, tcp_error)
    {
        LogInfo("Connecting to " << peer);
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

    void on_connected(network::tcp::socket& sock, network::endpoint const& peer)
    {
        LogInfo("Connected to " << peer);
        srv_ = in_place(boost::ref(sock), boost::bind(&client::on_receive, this, _1, _2), &tcp_error, &tcp_error);
        
        binary::bytes_t bts =  /*std::move(*/wrap_msg(setup(1111,2222))/*)*/;
        // srv_->send(&bts[0], bts.size());
        send(&bts[0], bts.size());
        // srv_->read();
    }

    void on_receive(const void* msg, size_t size)
    {
        //LogInfo("Some data received");
#if 0
        auto cmsg = static_cast<const char*>(msg);
        bytes_t data(cmsg, cmsg + size);

        auto it = data.begin();
        while (it != data.end())
        {
            auto new_it = find(it, data.end(), '\n');

            partial_msg_.insert(partial_msg_.end(), it, new_it);
            it = new_it;

            if (it != data.end())
            {
                ++it;

                string str(partial_msg_.begin(), partial_msg_.end());
                partial_msg_.clear();

                vector<string> strs;

                boost::split(strs, str, boost::is_any_of("\t\n"), boost::token_compress_on);
                Verify(strs.size() == 2);

                on_new_msg(strs[0], strs[1] == "on");
            }
        }
#endif
    }

    void on_new_msg(std::string const& adf_name, bool on)
    {   
        //printf(
        //    "=================  ADF name: %S; status: %s ====================\n", 
        //    unicode::utf8to16(adf_name).c_str(), 
        //    (on ? "turned on" : "turned off"));
    }

private:
    async_connector         con_;
    optional<tcp_socket>    srv_;

    bytes_t                 partial_msg_;
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

