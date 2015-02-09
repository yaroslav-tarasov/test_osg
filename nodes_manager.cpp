#include "stdafx.h"

#include "nodes_management.h"
#include "nodes_manager.h"
#include "find_node_visitor.h" 
#include "node_impl.h"

namespace nodes_management
{   
    class manager_impl;
    
    manager_ptr create_manager(osg::Node* node) 
    {
        return boost::make_shared<manager_impl>(node);
    }

    class manager_impl : public manager
    {
    public: 
        manager_impl( osg::Node* base )
            :base_(base)
        {}

        node_info_ptr find_node(std::string const& name) const override;
        node_info_ptr get_node    (uint32_t node_id)   const  override;
    private: 
        osg::ref_ptr<osg::Node> base_;
    };

    node_info_ptr manager_impl::find_node(std::string const& name) const
    {
        //for (auto it = nodes_.begin(); it != nodes_.end(); ++it)
        //    if ((*it)->data().name == name)
        //        return (*it);
        auto n = findFirstNode(base_,name,findNodeVisitor::not_exact);
        if(n)
            return boost::make_shared<node_impl>(n);

        return node_info_ptr();
    }

    node_info_ptr   manager_impl::get_node    (uint32_t node_id)   const
    {
          
          if(node_id==0)
                  return boost::make_shared<node_impl>(base_);   // FIXME ну бред же

          return node_info_ptr();
    }

}