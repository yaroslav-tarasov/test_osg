#ifndef AnimationLoader_h__
#define AnimationLoader_h__

namespace osg
{
class Texture2D ;
}

namespace avAnimation 
{

class AnimationLoader
{
public:

    // For loading Bones tree
    virtual void  AddBone ( const std::string& Name ) = 0;  
    virtual void  SetBoneInitialMatrix ( const osg::Matrixf& M ) = 0;
    virtual void  MoveLevelDown ( ) = 0;
    virtual void  MoveLevelUp ( ) = 0;

    // For loading additional Bones and setting ModelToBoneMatrix
    virtual void  SetIndexAndModelToBoneMatrix ( const std::string& Name, const osg::Matrixf& Mat ) = 0;
    // For loading Drawables
    virtual void  AddDrawable( const std::string& Name, std::vector<osg::Vec3f>& Vertices, std::vector<osg::Vec3f>& Normals,
                               std::vector<osg::Vec2f>& TxtCoord, std::vector<int>& Indices,
                               std::map<std::string, std::vector<std::pair<int,float>>>& BonesMapOfVerticesWeghtsTables,
                               const std::string& TextureName
                             ) = 0;

    // For building animations
    virtual void BeginAnimationSet ( const std::string& AnimationSetName ) = 0;
    virtual void BeginAnimation ( const std::string& BoneName, int FramesNum ) = 0;
    virtual void AddFrame ( int Frame, const osg::Matrixf& osgM ) = 0;
    virtual void EndAnimation ( ) = 0;
    virtual void EndAnimationSet ( ) = 0;
};

}

#endif // AnimationLoader_h__