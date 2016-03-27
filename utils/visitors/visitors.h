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


}