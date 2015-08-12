#include "stdafx.h"

#include "av/Scene.h"
#include "creators.h"

#include "async_services/async_services.h"
#include "network/msg_dispatcher.h"
#include "common/test_msgs.h"


using network::endpoint;
using network::async_acceptor;
using network::async_connector;
using network::tcp_socket;
using network::error_code;

using network::msg_dispatcher;
using network::tcp_fragment_wrapper;

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

struct server
{
    typedef boost::function<bool (double /*time*/)> on_timer_f;

    server(endpoint peer,on_timer_f on_timer,double period = 0.01)
        : acc_      (peer, boost::bind(&server::on_accepted, this, _1, _2), tcp_error)
        , period_   (period)
        , on_timer_ (on_timer)
        , timer_  (boost::bind(&server::update, this))
        
    {   

        disp_
            .add<setup                 >(boost::bind(&server::on_setup        , this, _1))
            .add<run                   >(boost::bind(&server::on_run        , this, _1))
            ;

        FIXME(Тут должно быть все гораздо сложнее);
        update();
    }

private:
    void update()
    {   
        if(!on_timer_( 0 ))
        {
           timer_.cancel();
           __main_srvc__->stop();
        }
        else
           timer_.wait(boost::posix_time::microseconds(int64_t(1e6 * period_)));
    }

    void on_accepted(network::tcp::socket& sock, endpoint const& peer)
    {
        //client_ = in_place(boost::ref(sock), network::on_receive_f(), boost::bind(&server::on_disconnected,this,_1)/*&tcp_error*/, &tcp_error);
        uint32_t id = next_id();
        sockets_[id] = std::shared_ptr<tcp_fragment_wrapper>(new tcp_fragment_wrapper(
            sock, boost::bind(&msg_dispatcher<uint32_t>::dispatch, &disp_, _1, _2, id),
            boost::bind(&server::on_disconnected, this, _1, id),
            boost::bind(&server::on_error, this, _1, id)));        
        
        LogInfo("Client " << peer << " accepted");
    }

    void on_disconnected(error_code const& ec)      
    {
         LogInfo("Client " << " disconnected with error: " << ec.message() );
    }
    
    void on_disconnected(boost::system::error_code const& ec, uint32_t sock_id)
    {  
        LogInfo("Client " << sock_id << " disconnected with error: " << ec.message() );
        sockets_.erase(sock_id);
        // peers_.erase(std::find_if(peers_.begin(), peers_.end(), [sock_id](std::pair<id_type, uint32_t> p) { return p.second == sock_id; }));
    }
    
    void on_error(boost::system::error_code const& ec, uint32_t sock_id)
    {
        LogError("TCP error: " << ec.message());
        sockets_.erase(sock_id);
        // peers_.erase(std::find_if(peers_.begin(), peers_.end(), [sock_id](std::pair<id_type, uint32_t> p) { return p.second == sock_id; }));
    }
    
    void on_setup(setup const& msg)
    {
         LogError("Got setup message: " << msg.srv_time << " : " << msg.task_time );
    }

    void on_run(run const& msg)
    {
        LogError("Got setup message: " << msg.srv_time << " : " << msg.task_time );
    }

    uint32_t next_id()
    {
        static uint32_t id = 0;
        return id++;
    }

private:
    async_acceptor          acc_   ;
    optional<tcp_socket>    client_;
    msg_dispatcher<uint32_t>  disp_;
    std::map<uint32_t, std::shared_ptr<tcp_fragment_wrapper> >  sockets_;

private:
    async_timer             timer_; 
    on_timer_f              on_timer_;
    double                  period_;

private:
    randgen<> rng_;
};

inline void initDataPaths()
{
    osgDB::getDataFilePathList().push_back(osgDB::getCurrentWorkingDirectory() + "\\data\\models");
    osgDB::getDataFilePathList().push_back(osgDB::getCurrentWorkingDirectory() + "\\data\\areas");
    osgDB::getDataFilePathList().push_back(osgDB::getCurrentWorkingDirectory() + "\\data\\materials");
    osgDB::getDataFilePathList().push_back(osgDB::getCurrentWorkingDirectory() + "\\data\\materials\\sky");
    osgDB::getDataFilePathList().push_back(osgDB::getCurrentWorkingDirectory() + "\\data\\materials\\lib");   
}


int av_scene( int argc, char** argv )
{
    osg::ArgumentParser arguments(&argc,argv);
    
    initDataPaths();
    
    logger::need_to_log(/*true*/);

#if 0
    // OSG graphics context
    osg::ref_ptr<osg::GraphicsContext::Traits> pTraits = new osg::GraphicsContext::Traits();
    //pTraits->inheritedWindowData           = new osgViewer::GraphicsWindowWin32::WindowData(hWnd);
    pTraits->alpha                         = 8;
    pTraits->setInheritedWindowPixelFormat = true;
    pTraits->doubleBuffer                  = true;
    pTraits->windowDecoration              = true;
    pTraits->sharedContext                 = NULL;
    pTraits->supportsResize                = true;
    pTraits->vsync                         = true;

    //RECT rect;
    //::GetWindowRect(hWnd, &rect);
    pTraits->x = 0;
    pTraits->y = 0;
    pTraits->width = 1920;//rect.right - rect.left + 1;
    pTraits->height = 1200;//rect.bottom - rect.top + 1;
#endif    

    async_services_initializer asi(false);
    
    logging::add_console_writer();

    __main_srvc__ = &(asi.get_service());

    try
    {
        endpoint peer(std::string("127.0.0.1:30000"));
        
        avScene::Scene::Create(arguments/*,pTraits*/);

        server s(peer,[](double)->bool{

            auto viewer = avScene::Scene::GetInstance()->GetViewer();

            if (!viewer->done())
            {
                viewer->frame();
                return true;
            }
            
            return false;

        });
        
        __main_srvc__->run();

    }
    catch(verify_error const&)
    {
        return -1;
    }
    catch(std::exception const& err)
    {
        std::string errror (err.what()); 
        LogError("Exception caught: " << err.what());
        return -1;
    }


    
    //asi.get_service().run();


#if 0
    //do
    //{
        avScene::Scene::Create(arguments/*,pTraits*/);
    
        auto viewer = avScene::Scene::GetInstance()->GetViewer();

        while (!viewer->done())
        {
             viewer->frame();
        }

        //avScene::Scene::Release();

    //} while (!avScene::Scene::zoneToReload().empty());
#endif


    return 0; //scene->GetViewer()->run();

}

AUTO_REG(av_scene)
