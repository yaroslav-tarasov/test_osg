#pragma once


#include <pugixml.hpp>

#include <iosfwd>

namespace avCore
{
namespace XML
{

typedef pugi::xml_parse_result  xml_parse_result ;
typedef pugi::xml_node          xml_node ;
typedef pugi::xml_attribute     xml_attribute ;

typedef pugi::xml_node_iterator xml_node_iterator ;
      
typedef pugi::xpath_node        xpath_node  ;
typedef pugi::xpath_node_set    xpath_node_set ;

struct XmlNode
: public xml_node
{
  typedef xml_node base ;

  XmlNode ()
  {}

  XmlNode ( xml_node node )
  : base ( node )
  {}

  template<typename T>
  inline T get_attribute_as ( char const * name ) const
  {
    return lexical_cast<T>( attribute ( name ).value ());
  }

  template<typename T>
  inline T get_attribute_as ( std::string const & name ) const
  {
    return lexical_cast<T>( attribute ( name.c_str ()).value ());
  }

  template<typename T>
  inline bool try_get_attribute_as ( T & value, char const * name ) const
  {
    xml_attribute attr = attribute ( name );

    if ( ! attr ) 
      return false ;

    char const * val = attr.value ();

    if ( ! val || '\0' == val [0] || '{' == val [0] )
      return false ;

    value = lexical_cast<T>( attr.value ());

    return true ;
  }

  template<>
  inline bool try_get_attribute_as ( std::string & value, char const * name ) const
  {
    xml_attribute attr = attribute ( name );

    if ( ! attr || ! attr.value ()) 
      return false ;

    value = lexical_cast<std::string>( attr.value ());

    return true ;
  }

  std::string getXml () const ;

  XML::XmlNode first_child ()
  {
    return base::first_child ();
  }

  XML::XmlNode next_sibling ()
  {
    return base::next_sibling ();
  }

  XML::XmlNode next_sibling ( const char * name )
  {
    return base::next_sibling ( name );
  }

  //*

  private :

    template<typename T>
    inline void set_attribute ( xml_attribute & attribute, T const & value ) const
    {
      attribute.set_value ( value );
    }

    template<>
    inline void set_attribute<std::string> ( xml_attribute & attribute, std::string const & value ) const
    {
      attribute.set_value ( value.c_str ());
    }

    template<>
    inline void set_attribute<unsigned __int64> ( xml_attribute & attribute, unsigned __int64 const & value ) const
    {
      char str [100];

      sprintf_s ( str, sizeof( str )/sizeof( str [0] ), "%I64u", value );

      attribute.set_value ( str );
    }

  public :

  //*
  template<typename T>
  inline void set_attribute_value ( char const * name, T const & value ) const
  {
    set_attribute ( attribute ( name ), value );
  }

  //*

  template<typename T>
  inline void append_attribute ( char const * name, T const & value )
  {
    xml_attribute attribute = base::append_attribute ( name );
    
    set_attribute ( attribute, value );
  }

  //*
  template<typename T>
  inline T get_child_value_as ( char const * name ) const
  {
    return lexical_cast<T>( child ( name ).first_child ().value ());
  }

  template<typename T>
  inline T get_value_as () const
  {
    return lexical_cast<T>( child_value ());
  }

  template<typename T>
  inline bool try_get_child_value_as ( T & value, char const * name ) const
  {
    xml_node node = child ( name ).first_child ();

    if ( ! node ) 
      return false ;

    char const * val = node.value ();

    if ( ! val || '\0' == val [0] || '{' == val [0] )
      return false ;

    value = lexical_cast<T>( val );

    return true ;
  }

  template<typename T>
  inline bool try_get_value_as ( T & value ) const
  {
    char const * val = child_value ();

    if ( ! val || '\0' == val [0] || '{' == val [0] )
      return false ;

    value = lexical_cast<T>( val );

    return true ;
  }


};

struct xml_document
{
  public :

    xml_document ();
    xml_document ( const char * first_child );

    xml_document ( xml_document const & );
    xml_document & operator = ( xml_document const & );

    bool loadFile ( const char * path );
    bool loadFile ( std::string const & path );

    bool saveFile ( const char * path ) const ;
    bool saveFile ( std::string const & path ) const ;

    bool loadXml  ( std::string const & xml );
    bool load     ( std::istream & is );

    operator XmlNode ();

    operator bool () const ;

    void operator = ( osg::Referenced * );

    std::string getXml () const ;

  private :

    void create ();

    osg::ref_ptr<osg::Referenced> _ref ;

    pugi::xml_document * _doc ;
};

typedef xml_document XmlDocument ;

}

}

