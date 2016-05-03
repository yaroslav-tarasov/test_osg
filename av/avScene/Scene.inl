

namespace avScene {

inline osgViewer::Viewer*                           Scene::GetViewer()     {  return _viewerPtr.get(); }
inline osg::Camera*                                 Scene::getCamera()     {  return _viewerPtr->getCamera(); }
inline osg::Group*                                  Scene::getCommonNode() { return _commonNode.get(); }  
inline avScene::Lights*                             Scene::getLights()     { return _lights.get(); }
inline std::vector<osg::ref_ptr<osg::Node>>&        Scene::getLamps()      { return _lamps; }
inline avWeather::Weather *                         Scene::getWeather()    { return _weatherPtr.get(); }
inline ScreenTextureManager *                       Scene::getScreenTextureManager() const { avAssert( _screenTextureManager.valid() ); return _screenTextureManager.get(); }
inline avLights::PointLightsManager *               Scene::getPointLightsManager() const { avAssert( _pointLightsManager.valid() ); return _pointLightsManager.get(); }

inline Scene* GetScene()
{
    return Scene::GetInstance();
}

}