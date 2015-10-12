#include <stdafx.h>

#include "av/avScene/ScreenTextureManager.h"


namespace avScene
{

ScreenTextureManager::ScreenTextureManager()
{
    setNodeMask(~65536); // disable reflection map pass
    setDataVariance(osg::Object::DYNAMIC);
}

ScreenTextureManager::~ScreenTextureManager()
{
}

ScreenTexture * ScreenTextureManager::request( unsigned order, ScreenTexture::Type type )
{
    const ScreenTexturePair id(order, type);
    const ScreenTextureMap::const_iterator iter = _depthTextureMap.find(id);
    osg::ref_ptr<ScreenTexture> texture;
    if (iter == _depthTextureMap.end())
    {
        texture = new ScreenTexture(order, type);
        _depthTextureMap[id] = texture;
    }
    else
        texture = iter->second;

    return texture.get();
}

void ScreenTextureManager::accept( osg::NodeVisitor & nv )
{
    if (!_depthTextureMap.empty() && nv.validNodeMask(*this))
    {
        nv.pushOntoNodePath(this);

        osgUtil::CullVisitor * cv = dynamic_cast<osgUtil::CullVisitor *>(&nv);
        if (cv != NULL)
        {
            for (ScreenTextureMap::const_iterator iter = _depthTextureMap.begin(); iter != _depthTextureMap.end(); ++iter)
            {
                ScreenTexture * texture = iter->second.get();
                cv->pushStateSet(texture->getStateSet());
                cv->addDrawable(texture, NULL);
                cv->popStateSet();
            }
        }
        else
            nv.apply(*this);

        nv.popFromNodePath();
    }
}


}
