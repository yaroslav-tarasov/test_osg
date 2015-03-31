#pragma once

namespace nodes_management
{
    struct node_info;
    struct node_control;
    struct vis_node_control;
    struct vis_node_info;
    struct manager;
    struct node_tree_iterator;
    struct visual;

    typedef polymorph_ptr<node_info>        node_info_ptr;
    typedef polymorph_ptr<node_control>     node_control_ptr;
    typedef polymorph_ptr<vis_node_control> vis_node_control_ptr;
    typedef polymorph_ptr<vis_node_info>    vis_node_info_ptr;
    typedef polymorph_ptr<manager>          manager_ptr;
    typedef polymorph_ptr<node_tree_iterator>   node_tree_iterator_ptr;  
    typedef polymorph_ptr<visual>           visual_manager_ptr;
    
}