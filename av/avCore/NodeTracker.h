#pragma once 

namespace utils
{

	class  NodeTracker : public osg::Object
	{
	public:
		NodeTracker(){};
		NodeTracker(const NodeTracker& nt, const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY){};

		META_Object(utils, NodeTracker);

		void             setTrackNode(osg::Node* node);
		osg::Node*       getTrackNode();
		const osg::Node* getTrackNode() const;

		__forceinline const osg::Matrix& getMatrix()    const {return _matrix;}
		__forceinline const osg::Matrix& getRotMatrix() const {return _rot_matrix;};   

		const osg::Matrix& update(/*osg::Node& node*/);

	private:
		void setTrackNodePath(const osg::NodePath& nodePath)         { _trackNodePath.setNodePath(nodePath); }
		void setTrackNodePath(const osg::ObserverNodePath& nodePath) { _trackNodePath = nodePath; }
		osg::ObserverNodePath& getTrackNodePath()                    { return _trackNodePath; }

	protected:

		osg::ObserverNodePath _trackNodePath;
		osg::Matrix           _matrix;
	    osg::Matrix           _rot_matrix;
	};

}