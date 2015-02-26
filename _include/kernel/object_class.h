#pragma once
#include "object_info_fwd.h"
#include "common/tinyxml_fwd.h"

namespace kernel
{

typedef ph_map<string, string>::multimap_t named_values_t ;

//! интерфейс "класса объекта" (вероятно класс в смысле предметной области?)
struct object_class
{
    virtual std::string         const&  name        () const = 0;
    virtual named_values_t      const&  attributes  () const = 0;
    virtual object_class_vector const&  classes     () const = 0;
    virtual object_class const* parent() const = 0;

    //! потрясающий комментарий :)
    // TODO: find out what these functions do!
    virtual object_class_ptr        find_class     (string const& name) const = 0;
    virtual boost::optional<string> find_attribute (string const& name) const = 0;
    virtual bool                    check_attribute(string const& name, string const &value) const = 0;

    virtual object_class_ptr search_class(string const& name) const = 0;

    virtual ~object_class(){}
};

//! создание иерархии по xml файлу
/*SYSTEMS_API*/ object_class_ptr create_object_class( tinyxml2::XMLElement const * node );

} // kernel
