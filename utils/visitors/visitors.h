#pragma once 

namespace Utils
{

template <class T>
class CommonVisitor : public osg::NodeVisitor
{
public:
    typedef std::function<void(T & nv)> apply_f;
public:
    CommonVisitor(apply_f fn)  
        : osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN)
        , fn_(fn)
    { }

    void apply( T& node )
    {
        fn_(node);
        traverse( node );
    }
private:
    apply_f  fn_;

};

struct FindGeometry : public osg::NodeVisitor
{
	osg::Drawable*  _geom;

	FindGeometry() : osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN) {}

	FindGeometry( osg::Node& node ) : osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN)
	{
		node.accept(*this);
	}

	void apply(osg::Geode& geode)
	{
		for (unsigned int i = 0; i < geode.getNumDrawables(); i++)
			apply(*geode.getDrawable(i));
	}

	void apply(osg::Drawable& geom)
	{
		_geom = &geom;
	}
};

}