#pragma once


inline void create_auto_object(kernel::system_ptr sys, std::string class_name, std::string unique_name)
{
    using namespace kernel;

    std::vector<object_class_ptr> const &classes = kernel::fake_objects_factory_ptr(sys)->object_classes() ;

    kernel::object_class_ptr class_ptr ;

    for (auto it = classes.begin(), end = classes.end(); it != end; ++it)
    {
        if (class_name == (*it)->name())
            class_ptr = *it ;
        std::string n = (*it)->name();
    }

    auto obj = kernel::fake_objects_factory_ptr(sys)->create_object(class_ptr, unique_name); 

}

FIXME("Не используется")
inline kernel::object_info_ptr create_object(kernel::system_ptr sys, std::string class_name, std::string unique_name)
{
    using namespace kernel;

    std::vector<object_class_ptr> const &classes = kernel::fake_objects_factory_ptr(sys)->object_classes() ;

    kernel::object_class_ptr class_ptr ;

    for (auto it = classes.begin(), end = classes.end(); it != end; ++it)
    {
        if (class_name == (*it)->name())
            class_ptr = *it ;
        std::string n = (*it)->name();
    }

    return kernel::fake_objects_factory_ptr(sys)->create_object(class_ptr, unique_name); 
}