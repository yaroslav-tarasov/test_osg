#pragma once

#include "thread_safe_queue.hpp"

namespace avCore
{
    struct  LoadNodeThread;

	class LoadManager 
		: public osg::Node
	{
        friend     struct  LoadNodeThread;
    public:
        typedef boost::function<osg::Node * ()> worker_f  ; 
        typedef boost::function<void ()       > set_signal_f  ;
	public:

		LoadManager();
        ~LoadManager();
		void           update  ( osg::NodeVisitor * nv );
		void           load    ( worker_f work, set_signal_f s );
        virtual bool   PreUpdate();

    private:
        struct Task;

		LoadNodeThread*                                     load_thread_;
        Utils::thread_safe_queue<Task>                          to_work_;
        Utils::thread_safe_queue<Task>                         finished_;
	};

}