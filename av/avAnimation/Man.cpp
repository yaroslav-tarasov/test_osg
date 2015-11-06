#include "stdafx.h"

#include "Man.h"

#include "avCore/avCore.h"
#include "avScene/Scene.h"
#include "avScene/Lights.h"

#include "avAnimation/AnimationObject.h"

#include <animation/animation.h>
#include <strstream>
#include <fstream>

#include "avCore/Database.h"
#include "avCore/XML.h"


namespace osg
{

struct StreamPtr : Stream
{
    StreamPtr ( std::auto_ptr<std::istream> & is ) : _is ( is ) {}

    virtual std::istream * get ()
    {
        return _is.get () ;
    }

    std::auto_ptr<std::istream> _is ;
};

}

namespace avAnimation 
{

class ManImpl
    : public Man
{
typedef Man base ;

public:

    ManImpl ( avCore::uint32 nID, avCore::XML::XmlNode pInfoXMLRootElement );
    ~ManImpl ();

    // object each-frame callback (called before OSG traversal, so you can freely modify scene graph)

    void update ( int TraversalNumber, double ReferenceTime );

private:

    virtual bool Initialize( const char* szName, const std::vector< std::string >& cDynamicNodes );
    virtual void Release();

private:

    virtual void setAnimation ( unsigned char const * data, size_t size )
    {
        QueuePtr queue = getQueue ();

        std::istrstream is ( reinterpret_cast<char const *>( data ), size );

        queue->read ( is );

        ::setAnimation ( _animation.get (), queue.get ());
    }
    
    virtual void setSubobjectCulling ( const char* Name, bool CullingIsOn )
    {
        if (CullingIsOn)
        {
            _animation->AddNameToCull(Name);
        }
        else
        {
            _animation->RemoveNameToCull(Name);
        }
    }

    virtual void placeTool ( unsigned uAttachPositionID, unsigned uToolID );

    virtual void clearAllTools ( );

  private :

    typedef osg::ref_ptr<avAnimation::AnimationObject> AnimationPtr ;
    AnimationPtr _animation ;

    // tools
    typedef std::map<unsigned, osg::ref_ptr<osg::Node>> ToolByID;
    ToolByID m_toolsMap;

    // attach positions
    struct tagAttachPosition
    {
        std::string sBoneName;
        osg::Vec3 vToolDisplace;
        avCore::TaitBryanRotation cToolRotation;
    };
    typedef std::map<unsigned, tagAttachPosition> AttachPosByID;
    AttachPosByID m_attachPosMap;

    // currently added tools
    typedef std::map<osg::ref_ptr<osg::Group>, std::vector<osg::ref_ptr<osg::Node>>> ToolsByGroup;
    ToolsByGroup m_currentlyPlacedTools;
};

//
// ManImpl
//

ManImpl::ManImpl( avCore::uint32 nID, avCore::XML::XmlNode pInfoXMLRootElement )
    : base ( nID )
{
    // Find all tools
    avCore::XML::XmlNode pToolsXMLElement = avCore::XMLReader::FindFirstChildElement(pInfoXMLRootElement, "Tools", avCore::XMLReader::DO_NOT_PRINT_ERROR);
    if (pToolsXMLElement)
    {
        avCore::XML::XmlNode pTool = pToolsXMLElement.child("Tool");
        while (pTool)
        {
            unsigned uID;
            if (avCore::XMLReader::GetUIntAttribute(pTool, "ID", uID, avCore::XMLReader::DO_NOT_PRINT_ERROR))
            {
                std::string sModelName;
                if (avCore::XMLReader::GetTextAttribute(pTool, "VisualModel", sModelName, avCore::XMLReader::DO_NOT_PRINT_ERROR))
                {
                    // try to add model to the map yo
                    osg::ref_ptr<osg::Node> pToolNode = avCore::Database::GetInstance()->LoadModel(sModelName.c_str());
                    if (pToolNode.valid())
                        m_toolsMap[uID] = pToolNode;
                }
            }
            pTool = pTool.next_sibling("Tool");
        }
    }

    // Find all tools positions
    avCore::XML::XmlNode pToolsPosXMLElement = avCore::XMLReader::FindFirstChildElement(pInfoXMLRootElement, "ToolsAttachPositions", avCore::XMLReader::DO_NOT_PRINT_ERROR);
    if (pToolsPosXMLElement)
    {
        avCore::XML::XmlNode pToolPos = pToolsPosXMLElement.child("ToolsAttachPosition");
        while (pToolPos)
        {
            unsigned uID;
            if (avCore::XMLReader::GetUIntAttribute(pToolPos, "ID", uID, avCore::XMLReader::DO_NOT_PRINT_ERROR))
            {
                tagAttachPosition newPos;
                if (avCore::XMLReader::GetTextAttribute(pToolPos, "Bone", newPos.sBoneName, avCore::XMLReader::DO_NOT_PRINT_ERROR))
                {
                    m_attachPosMap[uID] = newPos;
                }
            }
            pToolPos = pToolPos.next_sibling("ToolsAttachPosition");
        }
    }
}

ManImpl::~ManImpl()
{
}

inline ResourceLoaderPtr getDatabaseResourceLoader ()
{
    struct DatabaseResourceLoader
        : public ResourceLoader
    {
        virtual osg::ref_ptr<osg::Image> getImage ( std::string const & name ) 
        {
            return avCore::Database::GetInstance()->LoadImage ( name.c_str ());
        }

        virtual osg::ref_ptr<osg::Shader> getShader ( std::string const & name, osg::Shader::Type type )
        {
            return avCore::GetDatabase()->LoadShader ( name.c_str (), NULL, type );
        }

        virtual osg::ref_ptr<osg::Stream> getStream  ( std::string const & name )
        {
            std::string fileName;
            if ( !avCore::Database::GetInstance()->FindFile ( avCore::Database::PATH_MODEL, name.c_str(), & fileName ))
            {
                // avLog ( MT_INFO, "Can't find file '%s'.\n", name.c_str() );
                return NULL;
            }

            std::auto_ptr<std::istream> is = std::auto_ptr<std::istream> ( new std::ifstream ( fileName.c_str ()));
            if ( ! is->good ())
            {
                // avLog ( MT_INFO, "Can't open file '%s'.\n", fileName.c_str() );
                return NULL;
            }

            return new osg::StreamPtr ( is ) ;
        }
    };

    return ResourceLoaderPtr ( new DatabaseResourceLoader );
}

bool ManImpl::Initialize ( const char* szName, const std::vector< std::string >& cDynamicNodes )
{
    // prepare model
    _animation = avAnimation::CreateAnimationObject ( "F14.x", getDatabaseResourceLoader ());
    _animation->addCullCallback(new avScene::DynamicLightsObjectCull(avScene::LocalInfluence));
    _cVisualNodePtr->addChild ( _animation.get ());

    // exit
    return true;
}

void ManImpl::Release()
{
    clearAllTools();

    // Detach Man from the scene
    avScene::GetScene()->getCommonNode()->removeChild( this );
    base::Release();
}

void ManImpl::placeTool( unsigned uAttachPositionID, unsigned uToolID )
{
    ToolByID::iterator pToolIt = m_toolsMap.find(uToolID);
    if (pToolIt != m_toolsMap.end())
    {
        AttachPosByID::iterator pPosIt = m_attachPosMap.find(uAttachPositionID);
        if (pPosIt != m_attachPosMap.end())
        {
            const tagAttachPosition & curPos = pPosIt->second;
            osg::MatrixTransform * pGroupToAdd = _animation->AddTrackableBone(curPos.sBoneName.c_str());
            if (pGroupToAdd)
            {
                osg::Node * pToolNodePtr = pToolIt->second;
                // add
                pGroupToAdd->addChild(pToolNodePtr);
                m_currentlyPlacedTools[pGroupToAdd].push_back(pToolNodePtr);
            }
        }
    }
}

void ManImpl::clearAllTools( )
{
    ToolsByGroup::iterator pGroupToolsIt = m_currentlyPlacedTools.begin();
    for (ToolsByGroup::iterator pGroupToolsIt = m_currentlyPlacedTools.begin(); pGroupToolsIt != m_currentlyPlacedTools.end(); ++pGroupToolsIt)
    {
        osg::Group * pCurGroup = pGroupToolsIt->first;
        if (pCurGroup)
        {
            for (unsigned i = 0; i < pGroupToolsIt->second.size(); ++i)
                pCurGroup->removeChild(pGroupToolsIt->second[i]);
        }
    }
    m_currentlyPlacedTools.clear();
}

Man * CreateMan ( avCore::uint32 nID, avCore::XML::XmlNode pInfoXMLRootElement )
{
    return new ManImpl( nID, pInfoXMLRootElement );
}

}

