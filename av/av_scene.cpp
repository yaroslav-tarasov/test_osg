#include "stdafx.h"

#include "av/Scene.h"
#include "creators.h"

#include "async_services/async_services.h"

using network::endpoint;
using network::async_acceptor;
using network::async_connector;
using network::tcp_socket;
using network::error_code;

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
        client_ = in_place(boost::ref(sock), network::on_receive_f(), &tcp_error, &tcp_error);
        LogInfo("Client " << peer << " accepted");
    }

private:
    async_acceptor          acc_   ;
    optional<tcp_socket>    client_;

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


    
    asi.get_service().run();


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
