#pragma once

namespace utils
{
    // Use a thread to call osgDB::readNodeFile.
    struct  LoadNodeThread : public OpenThreads::Thread
    {
        typedef boost::function<void ()> on_work_f  ; 

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
            if( _work )
                _work();
        }

        on_work_f                 _work;
        osg::ref_ptr< osg::Node > _node;
    };

}