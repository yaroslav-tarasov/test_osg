#include "stdafx.h"

#include "ObjectManager.h"


namespace avCore
{

//////////////////////////////////////////////////////////////////////////
//
// ObjectManager
//
//////////////////////////////////////////////////////////////////////////
ObjectManager::~ObjectManager()
{
}

//////////////////////////////////////////////////////////////////////////
void ObjectManager::AddObject( ObjectBase* obj )
{
    avAssert( obj );
    int nType = obj->GetType();
    // avAssertMessage( _cObjectsMap[ nType ].find( obj->GetID() ) == _cObjectsMap[ nType ].end(), "Object with current ID has been already created" );
    _cObjectsMap[ nType ][ obj->GetID() ] = obj;
}

//////////////////////////////////////////////////////////////////////////
void ObjectManager::RemoveObject( ObjectBase* obj )
{
    avAssert( obj );
    int nType = obj->GetType();
    ObjectMap::iterator cIt = _cObjectsMap[ nType ].find( obj->GetID() );
    if ( cIt != _cObjectsMap[ nType ].end() )
    {
        _cObjectsMap[ nType ].erase( cIt );
    } else {
        avError( "Object not found in the map" );
    }
}

//////////////////////////////////////////////////////////////////////////
bool ObjectManager::PreUpdate()
{
    for ( uint16_t nMapType = 0; nMapType < ObjectBase::OBJECT_TYPE_MAX; nMapType++ )
    {
        for ( ObjectMap::iterator cIt = _cObjectsMap[ nMapType ].begin(); cIt != _cObjectsMap[ nMapType ].end(); cIt++ )
        {
            if ( cIt->second->PreUpdate() == false )
                return false;
        } 
    }

    return true;
}


//////////////////////////////////////////////////////////////////////////
ObjectManager* ObjectManager::GetInstance()
{
    static ObjectManager cManager;
    return &cManager;
}

}