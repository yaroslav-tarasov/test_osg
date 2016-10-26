#include "av/precompiled.h"

#include "av/avScene/Scene.h"
#include "av/avCore/Callbacks.h"

#include "LoadManager.h"

namespace avCore
{
    struct LoadManager::Task : public Referenced
    {
        Task ( worker_f w,  set_signal_f s)
            :   _worker    (w)
            ,   _sig       (s)
        {}

        worker_f                      _worker;
        set_signal_f                     _sig;
        osg::ref_ptr< osg::Node >       _node;
    };


    // Use a thread to call osgDB::readNodeFile.
    struct  LoadNodeThread : public OpenThreads::Thread
    {

        LoadNodeThread( LoadManager * lm )
            : running_ (true)
            , lm_(lm)
        {
            startThread();
        }

        ~LoadNodeThread()
        {}

        void run()
        {
            while(running_)
            {
                waiter_.wait(&getMutex());
                high_res_timer hr_timer;
                
                osg::ref_ptr<LoadManager::Task> cur_task;
                while ( (cur_task = lm_->to_work_.try_pop()) != nullptr )
                {
                    cur_task->_node = cur_task->_worker();
					//cur_task->_node->unref();
                    lm_->finished_.push(cur_task.release());
                }

               OSG_WARN << "LoadNodeThread::run : " << hr_timer.set_point() << "\n";
                
            }

        }

        void stop()
        {
              running_ = false;
              getCondition().broadcast();
        }

        OpenThreads::Mutex& getMutex()
        {
            static OpenThreads::Mutex   _mutex;
            return _mutex;
        }

        OpenThreads::Mutex& getUpdateMutex()
        {
            static OpenThreads::Mutex   _mutex;
            return _mutex;
        }

        OpenThreads::Condition& getCondition()
        {
            return waiter_;
        }

    private:
        LoadManager*                      lm_;
        OpenThreads::Condition        waiter_;
        bool                         running_;
    };




    LoadManager::LoadManager()
        : load_thread_(new LoadNodeThread(this))
    {
        // setUpdateCallback(Utils::makeNodeCallback(this, &LoadManager::update));
    }

    LoadManager::~LoadManager()
    {
         load_thread_->stop();
    }
    
    bool LoadManager::PreUpdate()
    {
        bool ret = true;
        
        update(nullptr);

        return ret;
    }

    // Use a thread to call osgDB::readNodeFile.
    void LoadManager::update( osg::NodeVisitor *  )
    {
        avScene::Scene * scene = avScene::GetScene();

        if (scene == NULL)
            return;
        
        double dt = 0;
        high_res_timer hr_timer;
 
       osg::ref_ptr<LoadManager::Task> cur_task = nullptr;
        while ( dt < 0.01 && ((cur_task = finished_.try_pop()) != nullptr))
        {
            if(cur_task->_node.valid())
            {
                //
                scene->getTerrainRoot()->addChild(cur_task->_node);
                cur_task->_sig();
            }

            dt += hr_timer.get_delta();
        }

		if(dt>0)
		{
			force_log fl;
			LOG_ODS_MSG("LoadManager::update( osg::NodeVisitor *  )" << dt << "\n");
		}
    }

    void LoadManager::load ( worker_f work , set_signal_f s)
    {
        //
        to_work_.push(new Task( work, s ));

        load_thread_->getCondition().broadcast();
    }

}