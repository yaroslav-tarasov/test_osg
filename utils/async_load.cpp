#include "stdafx.h"
#include "av/precompiled.h"

#include "av/avScene/Scene.h"
#include "async_load.h"
#include "utils/callbacks.h"

namespace Utils
{
    // Use a thread to call osgDB::readNodeFile.
    struct  LoadNodeThread : public OpenThreads::Thread
    {

        LoadNodeThread( LoadManager::worker_f work, LoadManager::set_signal_f s )
            : _worker( work )
            , _node(nullptr)
            , _sig (s)
        {
            high_res_timer hr_timer;
            startThread();
            OSG_WARN << "LoadNodeThread:: LoadNodeThread : " << hr_timer.set_point() << "\n";
        }

        ~LoadNodeThread()
        {}

        void run()
        {
            // 
            // OpenThreads::ScopedLock<OpenThreads::Mutex> lock(getMutex());
            high_res_timer hr_timer;

            if( _worker )
                _node = _worker();

            OSG_WARN << "LoadNodeThread::run : " << hr_timer.set_point() << "\n";
        }

        static OpenThreads::Mutex& getMutex()
        {
            static OpenThreads::Mutex   _mutex;
            return _mutex;
        }

        LoadManager::worker_f                      _worker;
        LoadManager::set_signal_f                     _sig;

        osg::ref_ptr< osg::Node >                    _node;

    };



    LoadManager::LoadManager()
    {
        setUpdateCallback(Utils::makeNodeCallback(this, &LoadManager::update));
    }

    // Use a thread to call osgDB::readNodeFile.
    void LoadManager::update( osg::NodeVisitor *  )
    {
        avScene::Scene * scene = avScene::GetScene();

        if (scene == NULL)
            return;

        for (auto it = threads_.begin(); it!= threads_.end();++it)
        {
            if(!(*it)->isRunning())
            {
               if((*it)->_node.valid())
               {
				   scene->getTerrainRoot()->addChild((*it)->_node);
				   (*it)->_sig();
			   }
               delete *it;
               threads_.erase(it);
               break;
            }
        }

        
        
    }

    void LoadManager::load ( osg::MatrixTransform* mt, worker_f work , set_signal_f s)
    {
        threads_.push_back(new LoadNodeThread(work,s));
    }

}