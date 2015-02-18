#include "client.h"
#include <QApplication>


using namespace boost;
using std::string;
using std::vector;
using boost::property_tree::ptree;
using boost::property_tree::info_parser::read_info;

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


client_interface_ptr create_client()
{
    const string config_filename = "sample_client.info";
    std::ifstream stream(config_filename);
    if (!stream.is_open())
    {
        LogError("can't read config");
        return client_interface_ptr();
    }

    ptree config;
    read_info(stream, config);

    const auto c = boost::make_shared<client>(config);

    const auto is_ready = [c]() { return c->is_ready(); };
    const bool res = sync_wait_for_qt(is_ready, pt::seconds(5));

    if (!res)
    {
        LogError("atc data client timeout");
        return client_interface_ptr();
    }

    return c;
}
