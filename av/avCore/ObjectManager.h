#pragma once 

#include <vector>
#include <osg/ref_ptr>
#include <osg/Node>


namespace avCore
{

    //////////////////////////////////////////////////////////////////////////
    // ObjectBase
    //////////////////////////////////////////////////////////////////////////
    class ObjectBase
    {

    public:
        enum ObjectType {
            OBJECT_STATIC		= 0,
            OBJECT_DYNAMIC		= 1,
            OBJECT_TYPE_MAX											// Always at the end
        };

    private:
        ObjectType				_eObjectType;
        uint32_t					    _nID;

    private:
        ObjectBase( ObjectType eType, uint32_t nID );
    public:
        virtual					~ObjectBase();

        virtual bool			Initialize();
        virtual bool			PreUpdate();

        inline uint32_t			GetID() const { return _nID; }
        inline ObjectType		GetType()     { return _eObjectType; }

    };

    //////////////////////////////////////////////////////////////////////////
    // ObjectManager
    //////////////////////////////////////////////////////////////////////////
    class ObjectManager
    {
    private:
        typedef std::map< uint32_t, ObjectBase* >	ObjectMap;
        ObjectMap				_cObjectsMap[ ObjectBase::OBJECT_TYPE_MAX ];

    public:
        ~ObjectManager();

        void					AddObject   ( ObjectBase* obj );
        void					RemoveObject( ObjectBase* obj );
        bool					PreUpdate();

        static ObjectManager*	GetInstance();
    };

#define GetObjectManager() ObjectManager::GetInstance()

}