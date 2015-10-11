#include <stdafx.h>

#include "avCore/detail/attributes.h"

#include <pugixml.hpp>

#include <iostream>
#include <sstream>

#ifdef _DEBUG
  #ifdef _CRTDBG_MAP_ALLOC
    #define new new (_NORMAL_BLOCK, __FILE__, __LINE__ )
  #endif
#endif

namespace avCore
{
namespace XML
{

struct xml_document_impl
: osg::Referenced
, pugi::xml_document
{
  xml_document_impl ()
  {}
};


xml_document::xml_document ()
: _doc ( 0 )
{}

xml_document::xml_document ( const char * first_child )
: _doc ( 0 )
{
  create ();

  _doc->append_child ( first_child );
}

xml_document::xml_document ( xml_document const & doc )
: _ref ( doc._ref )
, _doc ( doc._doc ) 
{}

void xml_document::create ()
{
  if ( _doc )
    return ;

  osg::ref_ptr<xml_document_impl> ptr = new xml_document_impl ;

  _ref = ptr.get () ;
  _doc = ptr.get () ;
}

xml_document & xml_document::operator = ( xml_document const & doc )
{
  _ref = doc._ref ;
  _doc = doc._doc ;

  return *this ;
}

void xml_document::operator = ( osg::Referenced * ptr )
{
  xml_document_impl * doc = dynamic_cast<xml_document_impl *> ( ptr );

  _ref = doc ;
  _doc = doc ;
}

xml_document::operator XmlNode ()
{
  if ( 0 != _doc )
    return static_cast<pugi::xml_node &>( *_doc ).first_child ();
  else
  {
    create ();

    return static_cast<pugi::xml_node &>( *_doc );
  }
}

xml_document::operator bool () const
{
  return 0 != _doc && ! static_cast<pugi::xml_node &>( *_doc ).first_child ().empty ();
}

std::string xml_document::getXml () const
{
	std::ostringstream os ;

  _doc->save ( os );

  return os.rdbuf ()->str ();
}

bool xml_document::loadFile ( std::string const & path )
{
  return loadFile ( path.c_str ());
}

bool xml_document::load ( std::istream & is )
{
  create ();

  xml_parse_result result = _doc->load ( is );

  if ( result )
    return true ;
  
  std::cerr << "XmlDocument: error loading XML from stream : " << result.description () << std::endl ;

  return false ;
}

bool xml_document::loadFile ( const char * path )
{
  create ();

  xml_parse_result result = _doc->load_file ( path );

  if ( result )
    return true ;
  
  std::cerr << "XmlDocument: error parsing file " << path << " : " << result.description () << std::endl ;

  return false ;
}

bool xml_document::saveFile ( std::string const & path ) const
{
  return saveFile ( path.c_str ());
}

bool xml_document::saveFile ( const char * path ) const
{
  bool result = _doc->save_file ( path );

  if ( result )
    return true ;
  
  std::cerr << "XmlDocument: error saving file " << path << std::endl ;

  return false ;
}

bool xml_document::loadXml ( std::string const & xml )
{
  create ();

  xml_parse_result result = _doc->load ( xml.c_str ());

  if ( result )
    return true ;

  std::cerr << "XmlDocument: error parsing xml " << result.description () << std::endl ;

  return 0 ;
}

//*
std::string XmlNode::getXml () const
{
  std::basic_stringstream<char, std::char_traits<char>, std::allocator<char>> os ;

  print ( os );

  return os.rdbuf ()->str ();
}

}
}

