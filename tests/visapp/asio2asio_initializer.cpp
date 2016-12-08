#include "async_services/async_services.h"
#include "asio2asio_initializer.h"
#include "asio2asio_dispatcher.h"

asio2asio_initializer::asio2asio_initializer()
{
    network::asio2asio::raw_disp() = in_place();
}

asio2asio_initializer::~asio2asio_initializer()
{
    network::asio2asio::raw_disp().reset();
}

boost::asio::io_service& asio2asio_initializer::get_service()
{
    return network::asio2asio::disp().get_service();
}

void asio2asio_initializer::run_main()
{
	boost::asio::io_service::work w(get_service());
	get_service().run();
}