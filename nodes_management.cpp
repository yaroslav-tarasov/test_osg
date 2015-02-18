#include "stdafx.h"
#include "nodes_management.h"
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
        if(func_(boost::make_shared<nm::node_impl>(&searchNode)))
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

cg::transform_4 get_relative_transform(manager_ptr manager, node_info_ptr node, node_info_ptr rel )
{
    osg::Matrix tr;
    osg::Node* n = node_impl_ptr(node)->as_osg_node();
    auto root = node_impl_ptr(manager->get_node(0))->as_osg_node();

    while(/*n->position().is_local() &&*/ 0 != n->getNumParents() && n != root && (rel.get()?n != node_impl_ptr(rel)->as_osg_node():true)  )
    {
        if(n->asTransform())
        if(n->asTransform()->asMatrixTransform())
            tr = n->asTransform()->asMatrixTransform()->getMatrix() * tr;

        n = n->getParent(0);
    }

    if (rel.get() == NULL || n == node_impl_ptr(rel)->as_osg_node()  )
        return from_osg_transform(tr);

    osg::Matrix tr_rel;
    n = node_impl_ptr(rel)->as_osg_node();
    while(/*n->position().is_local()*/0 != n->getNumParents() && n->getName() != root->getName())
    {                  
        if(n->asTransform())
        if(n->asTransform()->asMatrixTransform())
        tr_rel = n->asTransform()->asMatrixTransform()->getMatrix() * tr_rel;
        n = n->getParent(0);
    }

    return from_osg_transform((osg::Matrix::inverse(tr_rel)) * tr);
}


}
