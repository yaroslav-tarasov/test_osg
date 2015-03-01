#pragma once

#include "objects/nodes_management.h"

namespace nodes_management
{

    struct node_tree_iterator_impl
        : node_tree_iterator
    {   
        node_tree_iterator_impl( node_info_ptr self, manager const* man );
        node_tree_iterator_impl( node_info_ptr self );

        // node_tree_iterator
    public:
        std::vector<node_tree_iterator_ptr> const& children() const;
        node_info_ptr node() const;

    public:
        void add_child(node_tree_iterator_ptr child);


    private:
        std::vector<node_tree_iterator_ptr> children_;
        node_info_ptr self_;
    };
}