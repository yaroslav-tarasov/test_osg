#include "stdafx.h"
#include "objects/nodes_management.h"
#include "node_impl.h"


namespace nodes_management
{

class func_visitor : public osg::NodeVisitor 
{
public: 

    explicit func_visitor(std::function<bool(nm::node_info_ptr)> f)    
        : osg::NodeVisitor(TRAVERSE_ALL_CHILDREN)
        , func_(f)
    {}    

    virtual void apply(osg::Node &searchNode)
    {   
        FIXME (Nodes manager надо реализовывать нормально)
        if(func_(boost::make_shared<nm::node_impl>(&searchNode,nullptr)))
            traverse(searchNode);    
    }

private:
    std::function<bool(nm::node_info_ptr)> func_;
};

void visit_sub_tree(nm::node_info_ptr root,std::function<bool(nm::node_info_ptr)> f)
{
       func_visitor fv(f);
       nm::node_impl_ptr(root)->as_osg_node()->accept(fv);
};


}
