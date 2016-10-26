#pragma once

namespace Utils
{
    struct  LoadNodeThread;

	class LoadManager 
		: public osg::Node
	{
    public:
        typedef boost::function<osg::Node * ()> worker_f  ; 
        typedef boost::function<void ()       > set_signal_f  ;
	public:

		LoadManager();
		void   update  ( osg::NodeVisitor * nv );
		void   load    ( osg::MatrixTransform* mt, worker_f work, set_signal_f s );

		std::deque<LoadNodeThread*>               threads_;
	};

}