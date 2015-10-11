#include "stdafx.h"
#include "av/precompiled.h"


#include "NavAid.h"

//
// Module namespace
//

namespace avScene
{
    osg::ref_ptr<osg::Texture2D>  NavAidGroup::tex_;

    NavAidGroup::NavAidGroup (bool usePointSprites)
        : usePointSprites_(usePointSprites)
    {
       if(usePointSprites_ && !tex_.valid())
       {
           tex_ = new osg::Texture2D();
           tex_->setImage(osgDB::readImageFile("Images/particle.rgb"));
       }

       _createStateSet();
    }

    // copy constructor
    NavAidGroup::NavAidGroup( const NavAidGroup & rhs, const osg::CopyOp & copyop )
        : osgSim::LightPointNode(rhs, copyop)
        , lm_(rhs.lm_)
        , usePointSprites_(rhs.usePointSprites_)
    {
    }
    
    void  NavAidGroup::addLight(osgSim::LightPoint pnt, const LightManager::Light& data)
    {
        const uint32_t ind = addLightPoint(pnt);
		
		LightManager::Light d = data;

		d.color.r = pnt._color.r();
		d.color.g = pnt._color.g();
		d.color.b = pnt._color.b();
		
		assert(d.color.r <= 1.0 && d.color.g <= 1.0 && d.color.b <= 1.0);

        const uint32_t uid = avScene::LightManager::GetInstance()->addLight(d); 
		
		lm_.insert(std::make_pair(ind,uid));
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
    
    void NavAidGroup::_createStateSet()
    {
        //
        osg::StateSet* set = getOrCreateStateSet();

        if (usePointSprites_)
        {
            setPointSprite();

            // Set point sprite texture in LightPointNode StateSet.

            set->setTextureAttributeAndModes(0, tex_, osg::StateAttribute::ON);
#if 0                    
            osg::ref_ptr<osg::Program> cLightPointProg = creators::createProgram("simlight").program; 
            cLightPointProg->setName("LightLayerShader");
            set->setAttribute(cLightPointProg.get());
#endif
        }


        // set render bin
        set->setRenderBinDetails(RENDER_BIN_LIGHTS, "RenderBin");
        set->setNestRenderBins(false);
    }
}