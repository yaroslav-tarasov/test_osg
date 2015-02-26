#include "stdafx.h"

#include "object_class_impl.h"

#include "xml/xml.h"

namespace kernel
{

object_class_impl::object_class_impl( tinyxml2::XMLElement const * node )
    : parent_( NULL )
    , name_("root")
{
    std::queue<tinyxml2::XMLElement const*> q;
    q.push(node);

    while(!q.empty())
    {
        tinyxml2::XMLElement const* n = q.front();
        q.pop();
        if (n->Name() == std::string("units"))
        {
            for (tinyxml2::XMLElement const * ptr = n->FirstChildElement(); ptr; ptr = ptr->NextSiblingElement())
            {
                if (ptr->Name() == std::string("unit"))
                    classes_.push_back(create_object_class(this, ptr));
                else
                {
                    q.push(ptr);
                }
            }
        }
    }
}

object_class_impl::object_class_impl( object_class const* parent, tinyxml2::XMLElement const * node )
    : parent_( parent )
    , name_  (get_attribute(node, "name") )
{
    if (optional<string> ancestors_str = ::check_attribute(node, "ancestor"))
    {
        vector<string> ancestors_vec;
        boost::split(ancestors_vec, *ancestors_str, boost::is_any_of(","));
        BOOST_FOREACH(string ancestor_str, ancestors_vec)
        {
            boost::trim(ancestor_str);
            if (object_class_ptr ancestor = parent->search_class(ancestor_str))
            {
                ancestors_.push_back(ancestor);

                classes_.insert(classes_.end(), ancestor->classes().begin(), ancestor->classes().end());
                attributes_.insert(ancestor->attributes().begin(), ancestor->attributes().end());
            }
            else
                throw std::runtime_error("Incorrect ancestor") ;
        }
    }

    for (tinyxml2::XMLElement const * i = node->FirstChildElement(); i; i = i->NextSiblingElement())
    {
        if (strcmp(i->Value(), "unit") == 0)
        {
            classes_.push_back(create_object_class(this, i));

            // check for replacement (keep order)
            for (size_t i = 0 ; i != classes_.size() - 1; i ++)
            {
                if (classes_[i]->name() == classes_.back()->name())
                {
                    std::swap(classes_[i], classes_.back());
                    classes_.pop_back();
                    break ;
                }
            }
        }
        else if (strcmp(i->Value(), "attribute") == 0)
        {
            attributes_.insert(std::make_pair(get_attribute(i, "name"), get_attribute(i, "value", ""))) ;
        }
    }
}


std::string const& object_class_impl::name() const
{
    return name_;
}

named_values_t const& object_class_impl::attributes() const
{
    return attributes_;
}

object_class_vector const& object_class_impl::classes() const
{
    return classes_;
}

object_class const* object_class_impl::parent() const
{
    return parent_;
}

object_class_ptr object_class_impl::find_class( string const& name ) const
{
    const char * str = name.c_str();
    if ( str[0] == '\\' )
    {
        if ( parent_ )
            return parent_ -> find_class(name) ;

        return find_class( str + 1 ) ;
    }

    const char * p = strchr(str, '\\');
    if ( p == NULL )
    {
        BOOST_FOREACH( object_class_ptr cls, classes_)
        {
            if ( cls->name() == name )
                return cls;
        }
    }
    else
    {
        string fragment(str, p) ;
        BOOST_FOREACH( object_class_ptr cls, classes_)
        {
            if ( cls->name() == fragment )
                return cls->find_class(p + 1) ;
        }
    }

    return object_class_ptr();
}

optional<string> object_class_impl::find_attribute( string const& name) const
{
    named_values_t::const_iterator it = attributes_.find(name);
    if ( it != attributes_.end() )
        return it->second;

    return boost::none;
}

bool object_class_impl::check_attribute ( string const& name, string const &value ) const
{
    auto found = find_attribute(name) ;
    return found && (*found == value) ;
}

object_class_ptr object_class_impl::search_class( string const& name ) const
{
    if (object_class_ptr cl = find_class(name))
        return cl ;

    if (parent_)
        return parent_->search_class(name);

    return object_class_ptr();
}

object_class_ptr object_class_impl::create_object_class( object_class const* parent, tinyxml2::XMLElement const * node )
{
    return static_pointer_cast<object_class>(make_shared<object_class_impl>(parent, node));
}

//! создать ноду "дерева классов" на основе ноды файла xml; рекурсивно вызывает себя для дочерних нод
/*SYSTEMS_API*/ object_class_ptr create_object_class( tinyxml2::XMLElement const * node )
{
    return boost::make_shared<object_class_impl>(node);
}

} // kernel
