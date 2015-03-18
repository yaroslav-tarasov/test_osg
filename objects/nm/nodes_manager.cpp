#include "stdafx.h"
#include "precompiled_objects.h"

#include "objects/nodes_management.h"
#include "nodes_manager.h"
#include "node_impl.h"
#include "nodes_manager/nodes_manager_view.h"
#include "nodes_manager/node_tree_iterator.h"

namespace nodes_management
{   

    class manager_impl 
        :  public view 
    {
    public: 
        manager_impl( osg::Node* base, kernel::object_create_t const& oc, dict_copt dict) 
            : view(oc,dict)  
            , base_(base)
            , self_ (this)
            
        {}

        object_info_ptr manager_impl::create(osg::Node* base,kernel::object_create_t const& oc, dict_copt dict);

        node_info_ptr   find_node  (std::string const& name)                         const override;
        node_info_ptr   get_node   (uint32_t node_id)                                const  override;
        node_tree_iterator_ptr get_node_tree_iterator(uint32_t node_id) const override;
        cg::transform_4 get_relative_transform(/*manager_ptr manager,*/ node_info_ptr node, node_info_ptr rel ) override;
    private: 
        osg::ref_ptr<osg::Node> base_;
        manager_impl* const     self_;
    };

    node_info_ptr manager_impl::find_node(std::string const& name) const
    {
        if(base_)
        {
            auto n = findFirstNode(base_,name
                ,findNodeVisitor::not_exact);
            if(n)
                return boost::make_shared<node_impl>(n,self_);
        }
        else
           return view::find_node(name);

        return node_info_ptr();
    }

    node_info_ptr   manager_impl::get_node    (uint32_t node_id)   const
    {
          if(base_)
          {
              if(node_id==0)
                  return boost::make_shared<node_impl>(base_.get(), self_);   // FIXME ну бред же
          }
          else
          {
              return view::get_node(node_id);
          }

          return node_info_ptr();
    }

    object_info_ptr manager_impl::create(osg::Node* base,kernel::object_create_t const& oc, dict_copt dict)
    {
        manager_impl *obj = new manager_impl(base,oc, dict);
        object_info_ptr info(obj);
        obj->init();
        return info;
    }

    node_tree_iterator_ptr manager_impl::get_node_tree_iterator(uint32_t node_id) const
    {
        return boost::make_shared<node_tree_iterator_impl>(get_node(node_id), self_);
    }

    cg::transform_4 manager_impl::get_relative_transform(/*manager_ptr manager,*/ node_info_ptr node, node_info_ptr rel )
    {
        if(base_)
        {
            osg::Matrix tr;
            osg::Node* n = node_impl_ptr(node)->as_osg_node();
            auto root = node_impl_ptr(/*manager*/this->get_node(0))->as_osg_node();

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
        else
        {
            return view::get_relative_transform(node,rel);
        }
    }

    // FIXME —амо собой чушь
    void block_obj_msgs(bool block)
    {}

    void send_obj_message(size_t object_id, binary::bytes_cref bytes, bool sure, bool just_cmd)
    {} 

    manager_ptr create_manager(kernel::system_ptr sys, dict_copt dict, osg::Node* node) 
    {
        size_t id  = 0x666;
        //node_info_ptr nn = boost::make_shared<node_impl>(node,nullptr);
        //size_t id  = nn->root_node()->object_id();
        if(node)
        {
            osg::Node* root =  findFirstNode(node,"Root");
            if(root) 
                root->getUserValue("id",id);
        }

        auto msg_service = boost::bind(&send_obj_message, id, _1, _2, _3);
        auto block_msgs  = [=](bool block){ block_obj_msgs(block); };
        kernel::object_create_t  oc(
            nullptr, 
            sys.get(),                       // kernel::system*                 sys             , 
            id,                              // size_t                          object_id       , 
            "name",                          // string const&                   name            , 
            std::vector<object_info_ptr>(),  // vector<object_info_ptr> const&  objects         , 
            msg_service,                     // kernel::send_msg_f const&       send_msg        , 
            block_msgs                       // kernel::block_obj_msgs_f        block_msgs
            );

        manager_impl* obj = new manager_impl(node,oc,dict);
        manager_ptr man(obj);
        obj->init();

        return man/*boost::make_shared<manager_impl>(node,oc,dict)*/;
    }

}
