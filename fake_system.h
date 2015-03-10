#pragma once

namespace kernel
{

system_ptr create_model_system(msg_service& service, std::string const& script);

struct fake_objects_factory
{
    virtual ~fake_objects_factory(){}

    //virtual object_info_ptr create_object        (std::string const &object_name) = 0;
    virtual object_info_ptr create_object        (object_class_ptr hierarchy_class, std::string const &name)  = 0;
	virtual object_info_ptr create_object        (obj_create_data const& descr)                               = 0;
    virtual object_info_ptr load_object_hierarchy(dict_t const& dict)                                         = 0;
    virtual void            save_object_hierarchy(object_info_ptr objinfo, dict_t& dict, bool safe_key) const = 0;

    virtual object_class_vector const&  object_classes         ()                        const = 0;
    virtual object_class_ptr            get_object_class       (std::string const& name) const = 0;

};

typedef polymorph_ptr<fake_objects_factory>      fake_objects_factory_ptr;

}