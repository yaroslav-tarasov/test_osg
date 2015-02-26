#pragma once

#include "kernel/object_class.h"

namespace kernel
{
//! реализация "класса объекта" (???)
//! физически это содержимое objects.xml и подключаемых к нему других xml
struct object_class_impl 
    : object_class
{
    //! конструктор для корня дерева
    object_class_impl( tinyxml2::XMLElement const * node );
    //! конструктор для дочерней ноды дерева
    object_class_impl( object_class const* parent, tinyxml2::XMLElement const * node );

// object_class
public:
    //! геттер - имя этой ноды
    std::string         const& name             () const;
    //! геттер - все атрибуты
    named_values_t      const& attributes       () const;
    //! геттер - все дочерние классы
    object_class_vector const& classes          () const;
    //! геттер - родительская нода
    object_class const* parent() const;
    //! найти класс (ноду) по имени
    object_class_ptr find_class     ( string const& name ) const ;
    //! найти атрибут по имени неким хитрым способом (учитывая синтаксис имени)
    optional<string> find_attribute ( string const& name ) const ;
    //! проверить наличие атрибута с именем
    bool             check_attribute( string const& name, string const &value ) const ;
    //! найти класс по имени, базируется на find_class
    object_class_ptr search_class (string const& name) const;


public:
    static object_class_ptr create_object_class( object_class const * parent, tinyxml2::XMLElement const * node ) ;

private:
    object_class const* parent_;
    object_class_vector ancestors_;
    object_class_vector classes_;

    string name_;

    named_values_t  attributes_;
};

} // kernel
