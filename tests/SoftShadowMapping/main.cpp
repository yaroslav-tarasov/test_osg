#include <osg/AnimationPath>
#include <osg/PositionAttitudeTransform>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgGA/TrackballManipulator>
#include <osgShadow/ShadowedScene>
#include <osgShadow/SoftShadowMap>
#include <osgViewer/Viewer>

#include <fstream>
#include <iostream>

void printManip(osgGA::TrackballManipulator *m)
{
    osg::Vec3d eye;
    osg::Quat rot;
    m->getTransformation(eye, rot);
    printf("eye: %f %f %f\n",    eye.x(), eye.y(), eye.z());
    printf("rot: %f %f %f %f\n", rot.x(), rot.y(), rot.z(), rot.w());
}

std::string readShaderCode(const char *fileName)
{
    std::ifstream in(fileName);
    std::string s;
    std::string shaderCode;
    if (!in.is_open())
        return "";
    while (getline (in, s))
        shaderCode += s + "\n";
    return shaderCode;
}

osg::AnimationPath *createAnimationPath(float radius, float time)
{
    osg::ref_ptr<osg::AnimationPath> path = new osg::AnimationPath;
    path->setLoopMode(osg::AnimationPath::LOOP);
    unsigned int numSamples = 32;
    float delta_yaw = 2.0f * osg::PI / ((float)numSamples - 1.0f);
    float delta_time = time / (float)numSamples;
    for (unsigned int i = 0; i < numSamples; ++i)
    {
        float yaw = delta_yaw * (float)i;
        osg::Vec3 pos(sinf(yaw)*radius, cosf(yaw)*radius, 0.0f);
        osg::Quat rot(-yaw, osg::Z_AXIS);
        path->insert(delta_time * (float)i,
                     osg::AnimationPath::ControlPoint(pos, rot));
    }
    return path.release();
}

class ManipResetter : public osgGA::GUIEventHandler
{
    public:
        ManipResetter(osgGA::TrackballManipulator *m) : mManip(m) { }

        virtual bool handle(const osgGA::GUIEventAdapter &ea,
                            osgGA::GUIActionAdapter      &aa,
                            osg::Object                  *o,
                            osg::NodeVisitor             *nv)
        {
            switch (ea.getEventType())
            {
                case osgGA::GUIEventAdapter::KEYDOWN:
                {
                    if (ea.getKey() == 'x')
                    {
                        std::cout << "key down. resetting\n";
                        mManip->setTransformation(
                            osg::Vec3d(7.81569, 37.8102, -26.4537),
                            osg::Quat(-0.0235734, -0.487934, -0.867162, -0.0969207));
                        return true;
                    }
                    return false;
                }
                default:
                    return false;
            }
        }

    private:
        osgGA::TrackballManipulator *mManip;
};

int main(int argc, char *argv[])
{
    //osg::setNotifyLevel(osg::INFO);
    osg::setNotifyHandler(new osg::StandardNotifyHandler);
    // Ground.
    osg::ref_ptr<osg::MatrixTransform> ground = new osg::MatrixTransform;
    ground->addChild(osgDB::readNodeFile("lz.osg"));
    ground->setMatrix(osg::Matrix::translate(0, 0, -150));
    // Cessna.
    osg::ref_ptr<osg::MatrixTransform> cessna = new osg::MatrixTransform;
    cessna->addChild(osgDB::readNodeFile("cessna.osg.0,0,90.rot"));
    osg::ref_ptr<osg::AnimationPathCallback> apcb = new osg::AnimationPathCallback;
    apcb->setAnimationPath(createAnimationPath(50, 6));
    cessna->setUpdateCallback(apcb.get());
    // Truck.
    /*
    osg::ref_ptr<osg::MatrixTransform> truck = new osg::MatrixTransform;
    truck->addChild(osgDB::readNodeFile("dumptruck.osg"));
    truck->setMatrix(osg::Matrix::translate(0, 0, -50));
    */
    osg::ref_ptr<osg::PositionAttitudeTransform> truck = new osg::PositionAttitudeTransform;
    truck->addChild(osgDB::readNodeFile("dumptruck.osg"));
    truck->setPosition(osg::Vec3(0, 0, -50));
    // Light.
    osg::ref_ptr<osg::LightSource> source = new osg::LightSource;
    source->getLight()->setPosition(osg::Vec4(4, 4, 10, 0));
    source->getLight()->setAmbient(osg::Vec4(0.2, 0.2, 0.2, 1));
    source->getLight()->setDiffuse(osg::Vec4(0.8, 0.8, 0.8, 1));
    int shadowsize = 4096;//1024;
    osg::ref_ptr<osgShadow::SoftShadowMap> sm = new osgShadow::SoftShadowMap;
    sm->setTextureSize(osg::Vec2s(shadowsize, shadowsize));
    sm->setTextureUnit(1);
    sm->setJitteringScale(16);
    // Scene.
    osg::ref_ptr<osgShadow::ShadowedScene> root = new osgShadow::ShadowedScene;
    root->setShadowTechnique(sm.get());
    sm->setLight(source.get());
    root->addChild(ground.get());
    root->addChild(cessna.get());
    root->addChild(truck.get());
    root->addChild(source.get());
    // Box.
    osg::ref_ptr<osg::MatrixTransform> box = new osg::MatrixTransform;
    box->addChild(osgDB::readNodeFile("box.osgt"));
    box->setMatrix(osg::Matrix::translate(10, 2, -45));
    root->addChild(box.get());
    std::string vert = readShaderCode("/tmp/box.vert");
    std::string frag = readShaderCode("/tmp/box.frag");
    if (vert.length() && frag.length())
    {
        osg::ref_ptr<osg::Program> prog = new osg::Program;
        prog->setName("box shader");
        prog->addShader(new osg::Shader(osg::Shader::VERTEX,   vert));
        prog->addShader(new osg::Shader(osg::Shader::FRAGMENT, frag));
        osg::StateSet *ss = box->getChild(0)->getOrCreateStateSet();
        ss->setAttributeAndModes(prog, osg::StateAttribute::ON);
        std::cout << "set up box shader\n";
        osgDB::writeNodeFile(*box->getChild(0), "/tmp/box.osgx");
    }
    // Window.
    osg::ref_ptr<osg::GraphicsContext::Traits> traits =
        new osg::GraphicsContext::Traits;
    traits->x = 50;
    traits->y = 50;
    traits->width  = 800;
    traits->height = 600;
    traits->windowDecoration = true;
    traits->doubleBuffer = true;
    traits->samples = 4;
    osg::ref_ptr<osg::GraphicsContext> gc =
        osg::GraphicsContext::createGraphicsContext(traits.get());
    osg::ref_ptr<osg::Camera> camera = new osg::Camera;
    camera->setGraphicsContext(gc);
    camera->setViewport(
        new osg::Viewport(0, 0, traits->width, traits->height));
    camera->setClearMask(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    camera->setClearColor(osg::Vec4f(0.2, 0.2, 0.4, 1));
    camera->setProjectionMatrixAsPerspective(
        30,
        (double)traits->width / (double)traits->height,
        1,
        1000);
    // Viewer.
    osgViewer::Viewer viewer;
    viewer.setRunMaxFrameRate(40);
    viewer.setCamera(camera.get());
    viewer.setSceneData(root.get());
    // Manipulator.
    osg::ref_ptr<osgGA::TrackballManipulator> manip = new osgGA::TrackballManipulator;
    viewer.setCameraManipulator(manip);
    osg::ref_ptr<ManipResetter> mr = new ManipResetter(manip.get());
    viewer.addEventHandler(mr);
    // run.
    int r = viewer.run();
    printManip(manip.get());
    return r;
}
