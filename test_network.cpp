#include "stdafx.h" 

#include "async_services/async_services.h"
#include "async_services/async_timer.h"
#include "cpp_utils/thread_aware_obj.h"
#include "msg_dispatcher.h"

using namespace boost;
using namespace cg;
using std::string;
using std::vector;
using boost::property_tree::ptree;

enum message_id
{
    id_data_request,
    id_data_responce,
    id_places_request,
    id_places_response
};

namespace  aa
{
    struct data_t
    {

    };
}

struct client_interface
{

    virtual ~client_interface() {}
    //virtual atc::airport::data_t get_airport_data(const string &name) = 0;
    virtual bool is_ready() const = 0;

    enum place_t
    {
        CITY = 0,
        VILLAGE
    };

    virtual vector<cg::geo_point_2> get_places(const string &name, place_t type) = 0;
};

struct data_response
    : network::msg_id<id_data_responce>
{
    typedef aa::data_t data_t;

    data_response() {}
    data_response(data_t &&data)
        : data(std::forward<data_t>(data))
    {}

    data_t data;
};

struct data_request
    : network::msg_id<id_data_request>
{
    data_request() {}
    data_request(const string &name)
        : name(name)
    {}

    string name;
    typedef data_response response_t;
};



struct places_response
    : network::msg_id<id_places_response>
{
    typedef vector<cg::geo_point_2> data_t;

    places_response() {}
    places_response(data_t &&data)
        : data(std::forward<data_t>(data))
    {}

    data_t data;
};

struct places_request
    : network::msg_id<id_places_request>
{
    places_request() {}
    places_request(const string &name, client_interface::place_t type)
        : name(name)
        , type(type)
    {}

    string name;
    client_interface::place_t type;

    typedef places_response response_t;
};

REFL_STRUCT(data_request)
    REFL_ENTRY(name)
REFL_END()

REFL_STRUCT(data_response)
    REFL_ENTRY(data)
REFL_END()

REFL_STRUCT(places_request)
    REFL_ENTRY(name)
    REFL_ENTRY(type)
REFL_END()

REFL_STRUCT(places_response)
    REFL_ENTRY(data)
REFL_END()

class client_impl
    : public  boost::enable_shared_from_this<client_impl>
    , private boost::noncopyable
    , client_interface
{
private:
    typedef network::tcp::socket socket;
    typedef network::endpoint endpoint;
    typedef network::error_code error_code;
    typedef network::tcp_fragment_wrapper wrapper_t;
    typedef network::async_connector connector_t;

private:
    client_impl(pt::time_duration timeout);
    void init(const endpoint &ep);

public:
    static boost::shared_ptr<client_impl> create(const ptree &config);

    ~client_impl();
    void finalize();

public:
    //aa::data_t get_airport_data(const string &name);
    bool extract_responce_data();
    bool is_ready() const;
    std::vector<cg::geo_point_2> get_places(const string &name, place_t type);

private:
    void on_connected(network::tcp::socket& sock, endpoint const& ep);
    void on_refused(error_code const& error);
    void on_error(error_code const& error);
    void on_disconnected(error_code const& code);
    void on_receive(const void* data, size_t size);

    //void on_data_response(const data_response &response);

    static asio::io_service &service();

private:
    shared_ptr<connector_t> connector_;
    shared_ptr<wrapper_t> wrapper_;
    network::msg_dispatcher<> disp_;

    template<typename T>
    struct sync_getter;

    typedef sync_getter<data_request> data_getter_t;
    shared_ptr<data_getter_t> data_getter_;

    typedef sync_getter<places_request> places_getter_t;
    shared_ptr<places_getter_t> places_getter_;

    pt::time_duration timeout_;
};


bool sync_wait_for_qt(const boost::function<bool()> &fn, pt::time_duration timeout);

template<typename T>
struct client_impl::sync_getter
{
    typedef sync_getter<T> self_t;
    typedef T request_t;
    typedef typename request_t::response_t response_t;
    typedef typename response_t::data_t data_t;

    data_t get(shared_ptr<wrapper_t> wrapper, const request_t &request, pt::time_duration timeout)
    {
        if (!wrapper)
        {
            LogError("No connection");
            return data_t();
        }

        ready_ = false;
        //ready_.monitor().get() = false;
        
        const auto bytes = network::wrap(request);
        wrapper->send(binary::raw_ptr(*bytes), binary::size(*bytes));

        time_counter tc;
        const auto bind = boost::bind(&self_t::is_ready, this);
        const bool res = sync_wait_for_qt(bind, timeout);

        if (!res)
        {
            LogError("ATC data client timeout");
            return data_t();
        }

        return std::move(data_);
    }

    void on_response(const response_t &response)
    {
//         auto m = ready_.monitor();
//         m.get() = true;
        data_ = response.data;
        ready_ = true;
    }

private:
    bool is_ready() const
    {
        return ready_;
    }

private:
    thread_ops::thread_aware_obj<bool> ready_;
    data_t data_;
};


client_impl::client_impl(pt::time_duration timeout)
    : data_getter_  (make_shared<data_getter_t  >())
    , places_getter_(make_shared<places_getter_t>())
    , timeout_(timeout)
{
}

void client_impl::init(const endpoint &ep)
{
    connector_ = make_shared<connector_t>(
        ep, 
        boost::bind(&client_impl::on_connected, shared_from_this(), _1, _2),
        boost::bind(&client_impl::on_refused  , shared_from_this(), _1),
        boost::bind(&client_impl::on_error    , shared_from_this(), _1));
    
    
    //disp_.add<data_response>(boost::bind(&client_impl::on_data_response, shared_from_this(), _1));
    disp_.add<data_response  >(boost::bind(&data_getter_t  ::on_response, data_getter_  , _1));
    disp_.add<places_response>(boost::bind(&places_getter_t::on_response, places_getter_, _1));
}

shared_ptr<client_impl> client_impl::create(const ptree &config)
{
    const endpoint ep  = config.get<string>("addr");
    const auto timeout = config.get<unsigned>("timeout", 5);
    
    shared_ptr<client_impl> self(new client_impl(pt::seconds(timeout)));
    self->init(ep);
    return self;
}

client_impl::~client_impl()
{

}

void client_impl::finalize()
{
    wrapper_.reset();
    disp_ = network::msg_dispatcher<>();
    connector_.reset();
}

//aa::data_t client_impl::get_airport_data(const string &name)
//{
//    // Send request
//    const data_request request(name);
//    auto res = data_getter_->get(wrapper_, request, timeout_);
//    return std::move(res);
//}

vector<geo_point_2> client_impl::get_places(const string &name, place_t type)
{
    const places_request request(name, type);
    auto res = places_getter_->get(wrapper_, request, timeout_);
    return std::move(res);
}

/*
void client_impl::on_data_response(const data_response &response)
{
    auto m = airport_response_.monitor();
    m->data = response.data;
    m->ready = true;
}
*/



void client_impl::on_connected(network::tcp::socket& sock, endpoint const& ep)
{
    wrapper_ = make_shared<wrapper_t>(
        sock, 
        boost::bind(&client_impl::on_receive,      shared_from_this(), _1, _2),
        boost::bind(&client_impl::on_disconnected, shared_from_this(), _1),
        boost::bind(&client_impl::on_error,        shared_from_this(), _1));

    LogInfo("ATC data client connected to " << ep);
}

void client_impl::on_refused(error_code const& error)
{
    wrapper_.reset();
    LogError("Connection refused: " << error);
}

void client_impl::on_error(error_code const& error)
{                                                        
    LogError(error);
}

void client_impl::on_disconnected(error_code const& error)
{
    wrapper_.reset();
    LogError("Disconnected: " << error);
}

void client_impl::on_receive(const void *data, size_t size)
{
    if (!disp_.dispatch(data, size))
        LogError("Unexpected message received!");
}

asio::io_service &client_impl::service() 
{
    return async_services_initializer::get_service();
}

bool client_impl::is_ready() const
{
    return wrapper_ != NULL;
}

AUTO_REG(main_net)

int main_net(int argc, char** argv)
{
	async_services_initializer asi(false);

    asi.get_service().run();
	return 0;
}


bool sync_wait_for_qt(const boost::function<bool()> &fn, pt::time_duration timeout)
{
    time_counter time;
    while (!fn())
    {
        if (time.time() > timeout)
            return false;

        QCoreApplication::instance()->processEvents(QEventLoop::WaitForMoreEvents);
    }
    return true;
}