#include "AnimTest.h"

#include "XFileParser.h"

#include "av/avScene/Scene.h"
#include "av/avCore/Database.h"

#include "Animation.h"

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

typedef osg::ref_ptr<avAnimation::AnimationObject> AnimationPtr ;


osg::Group*  Initialize ( const char* szName )
{
    static AnimationPtr _animation ;

    osg::Group* node = new osg::Group;

    // prepare model
    _animation = avAnimation::CreateAnimationObject ( szName, getDatabaseResourceLoader ());
    //_animation->addCullCallback(new avScene::DynamicLightsObjectCull(avScene::LocalInfluence));
    node->addChild ( _animation.get ());

    // exit
    return node;
}


}