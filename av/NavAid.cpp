#include "stdafx.h"
#include "NavAid.h"


//
// Module namespace
//

namespace avScene
{
    NavAidGroup::NavAidGroup ()
    {
    }

    // copy constructor
    NavAidGroup::NavAidGroup( const NavAidGroup & rhs, const osg::CopyOp & copyop )
        : osgSim::LightPointNode(rhs, copyop)
    {
    }
    
    void  NavAidGroup::addLight(osgSim::LightPoint pnt, const LightManager::Light& data)
    {
        const uint32_t ind = addLightPoint(pnt);
        const uint32_t uid = avScene::LightManager::GetInstance()->genUID();
        lm_.insert(std::make_pair(ind,uid));
        avScene::LightManager::GetInstance()->addLight(uid, data);
    }

    void NavAidGroup::traverse(osg::NodeVisitor& nv)
    {
        osgSim::LightPointNode::traverse(nv);
        
        double time = nv.getFrameStamp()?nv.getFrameStamp()->getSimulationTime():0;

        LightPointList& list = getLightPointList(); 
        
        for(LightPointList::iterator itr=list.begin();
            itr!=list.end();
            ++itr)
        {
            const osgSim::LightPoint& lp = *itr;

            const uint32_t uid = lm_[std::distance(list.begin(),itr)];
            
            LightManager::Light&  light_data = LightManager::GetInstance()->getLight(uid);
            
            light_data.active = lp._on;

            if (!lp._on)
                continue;

            bool doBlink = lp._blinkSequence.valid();
            if (doBlink && _lightSystem.valid())
                doBlink = (_lightSystem->getAnimationState() == osgSim::LightPointSystem::ANIMATION_ON);

            if (doBlink)
            {
                FIXME(Интервал тут не просто так)
                osg::Vec4 bs = lp._blinkSequence->color(time,0/*timeInterval*/);
                if(bs.length2()>0)
                {
                    light_data.active = true;
                }
                else
                {
                    light_data.active = false;
                }
            }



        }

 
    }

}