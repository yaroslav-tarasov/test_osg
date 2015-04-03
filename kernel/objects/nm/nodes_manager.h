 #pragma once


namespace nodes_management
{
        manager_ptr create_manager(kernel::system_ptr sys,dict_copt dict,osg::Node* node);  
}