#pragma once

namespace utils
{
    // Use a thread to call osgDB::readNodeFile.
    struct  LoadNodeThread : public OpenThreads::Thread
    {
        typedef boost::function<osg::Node * ()> on_work_f  ; 

        LoadNodeThread( on_work_f work )
            : _work( work )
            , _node(nullptr)
        {
            startThread();
        }

        ~LoadNodeThread()
        {}

        void run()
        {

          OpenThreads::ScopedLock<OpenThreads::Mutex> lock(getMutex());

            if( _work )
               _node = _work();
        }

        static OpenThreads::Mutex& getMutex()
        {
            static OpenThreads::Mutex        _mutex;
            return _mutex;
        }

        on_work_f                 _work;
        osg::ref_ptr< osg::Node > _node;
        
    };

    class LoadManager 
        : public osg::Node
    {

    public:

        LoadManager();
        void   update  ( osg::NodeVisitor * nv );
        void   load    ( osg::MatrixTransform* mt, LoadNodeThread::on_work_f work );

        LoadNodeThread::on_work_f _work;

        std::deque<LoadNodeThread*>               threads_;
        std::deque<osg::ref_ptr< osg::Node >>   nodesToAdd;
    };
          
}