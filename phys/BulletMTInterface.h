#pragma once 

#include <osgbInteraction/SaveRestoreHandler.h>

namespace bi 
{

class BulletMTInterface : public osg::Referenced
{
public:
    typedef std::function<void(int id)> on_collision_f;
public:
    BulletMTInterface();
    static BulletMTInterface*  instance();
    //btDiscreteDynamicsWorld* getScene() { return _scene; }
    void createWorld( const osg::Plane& plane, const osg::Vec3& gravity, on_collision_f on_collision );
    void createCow( osg::Node* node,osg::Vec3 pos, const osg::Matrix& m, osg::Transform* amt );
    /*osg::Transform**/
    void makeModel( const std::string& fileName, const int index, osg::Vec3 pos,const osg::Matrix& m, osg::Transform* amt  );
    void registerHandlers(osg::Group* attachPoint,osgViewer::Viewer* viewer);
    void simulate();
    osgGA::GUIEventHandler* getSaveRestoreHandler();
private:
    struct private_data;
    osg::ref_ptr<private_data> d;
    on_collision_f                        _on_collision;
};

}