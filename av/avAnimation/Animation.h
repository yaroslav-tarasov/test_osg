#pragma once

namespace osg
{
struct Stream
: osg::Referenced 
{
  virtual std::istream * get () = 0 ;
};
}
  
namespace avAnimation 
{
struct ResourceLoader
: osg::Referenced
{
  virtual ~ResourceLoader ()
  {}
  
  virtual osg::ref_ptr<osg::Stream>   getStream ( std::string const & name ) = 0 ;
  virtual osg::ref_ptr<osg::Image>    getImage  ( std::string const & name ) = 0 ;
  virtual osg::ref_ptr<osg::Shader>   getShader ( std::string const & name, osg::Shader::Type type ) = 0 ;
};

typedef osg::ref_ptr<ResourceLoader> ResourceLoaderPtr ;

class AnimationObject : public osg::MatrixTransform
{
public:

    virtual void AddAnimationElement ( const std::string &AnimationName, float StartTime, float Duration, 
                                                                                    int RepeatTimes = 1, float BlendToNextTime = 0.0f, bool ClearTheQueue = false ) = 0;
    virtual void AddRoutedAnimationElement ( const std::string &AnimationName, float StartTime, float Duration, 
                                      const std::vector<osg::Vec3f> &route, float Speed, int RepeatTimes = 1, float BlendToNextTime = 0.0f, 
                                                                                                    const osg::Vec3f *ForwardDirection = 0 ) = 0;
    virtual void AddAnimationSubElement ( const std::string &AnimationName, float PauseLength, float Duration, 
                                                                                                        float BlendBorderLength = 0.0f ) = 0;
    virtual void RemoveAnimationElements ( ) = 0;
    virtual void SetAnimationElementsRepeat ( bool Repeat ) = 0;
    virtual void AddNameToCull ( const char *Name ) = 0;
    virtual void RemoveNameToCull ( const char *Name ) = 0;
    virtual bool IsNameCulled ( const char *Name ) const = 0;
    virtual void SetScale ( const osg::Vec3 &Scale ) = 0;

    virtual osg::MatrixTransform * AddTrackableBone( const char * szBone ) = 0;
};

AnimationObject *CreateAnimationObject ( const char *Model, ResourceLoaderPtr TextureLoader );

AnimationObject *CreateAnimationObjectFromFile ( const char *Model );

}
