#include "stdafx.h"
#include "precompiled_objects.h"

#include "node_tree_iterator.h"

namespace nodes_management
{
    node_tree_iterator_impl::node_tree_iterator_impl( node_info_ptr self, manager const* man )
        : self_(self)
    {
        typedef ph_map<size_t, node_tree_iterator_impl *>::map_t node_tree_iterator_map_t;

        node_tree_iterator_map_t map;
        map.insert(std::make_pair(self_->node_id(), this));

        man->visit_nodes([this, man, &map](node_info_ptr node)
        {  
            std::string name = node->name();

            node_position const& pos = node->position();
            if (pos.is_local() && pos.local().relative_obj == dynamic_cast<object_info const*>(man)->object_id())
            {
                auto it = map.find(pos.local().relative_node);
                if (it != map.end())
                {
                    auto nod_tree_it = boost::make_shared<node_tree_iterator_impl>(node);
                    it->second->add_child(nod_tree_it);

                    map.insert(std::make_pair(node->node_id(), nod_tree_it.get()));
                }
            }
        });
    }

    node_tree_iterator_impl::node_tree_iterator_impl( node_info_ptr self )
        : self_(self)
    {
    }

    std::vector<node_tree_iterator_ptr> const& node_tree_iterator_impl::children() const
    {
        return children_;
    }

    node_info_ptr node_tree_iterator_impl::node() const
    {
        return self_;
    }

    void node_tree_iterator_impl::add_child(node_tree_iterator_ptr child)
    {
        children_.push_back(child);
    }

}
