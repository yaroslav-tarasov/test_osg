#pragma once

namespace Utils
{
    // Use a thread to call osgDB::readNodeFile.
    struct  LoadNodeThread : public OpenThreads::Thread
    {
        typedef boost::function<osg::Node * ()> worker_f  ; 
		typedef boost::function<void ()       > set_signal_f  ;

        LoadNodeThread( worker_f work, LoadNodeThread::set_signal_f s )
            : _worker( work )
            , _node(nullptr)
			, _sig (s)
        {
            startThread();
        }

        ~LoadNodeThread()
        {}

        void run()
        {
			// 
            // OpenThreads::ScopedLock<OpenThreads::Mutex> lock(getMutex());

            if( _worker )
               _node = _worker();
        }

		static OpenThreads::Mutex& getMutex()
		{
			static OpenThreads::Mutex   _mutex;
			return _mutex;
		}

		worker_f                      _worker;
		osg::ref_ptr< osg::Node >       _node;
		set_signal_f                     _sig;



	};

	class LoadManager 
		: public osg::Node
	{

	public:

		LoadManager();
		void   update  ( osg::NodeVisitor * nv );
		void   load    ( osg::MatrixTransform* mt, LoadNodeThread::worker_f work, LoadNodeThread::set_signal_f s );

		std::deque<LoadNodeThread*>               threads_;
	};

}