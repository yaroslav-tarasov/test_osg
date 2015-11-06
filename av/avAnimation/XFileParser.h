#pragma once

#include <deque>
#include <iosfwd>
#include "AnimationLoader.h"

namespace avAnimation 
{

class Parser
{
   //typedef std::auto_ptr<std::istream> StreamPtr ;
  
public:
	 Parser ();
	~Parser ();

    bool            Load ( std::istream * is, AnimationLoader * Loader );
private:
    bool            IsEmpty();
    bool            GetNextToken( std::string &token );
    void            GetOutOfScope( int initial_scope_counter );
    void            GetBoneFrame();
    void            GetRootFrame();
    void            GetFrame();
    void            GetMesh( const std::string &name );
    void            GetMeshMaterialList( std::string &TextureName );
    void            GetAnimationSet();
    void            CheckNext( const char *Format );
    float           GetFloat();
    osg::Matrixf    GetMatrix();
    float           GetInt();
    void            Error( const char *Text );

    std::istream *  m_pStream ;
    
    std::deque<std::string> m_Tokens;
    int                     m_nLineNum;
    AnimationLoader*        m_pLoader;
};

}

