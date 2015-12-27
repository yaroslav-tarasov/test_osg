#pragma once

#include <osgAnimation/AnimationManagerBase>
#include <osgAnimation/Bone>

#include <osgAnimation/ActionStripAnimation>
#include <osgAnimation/ActionBlendIn>
#include <osgAnimation/ActionBlendOut>
#include <osgAnimation/ActionAnimation>
#include <osgAnimation/RigGeometry>
#include <osgAnimation/RigTransformHardware>
#include <osgAnimation/BoneMapVisitor>

#include "materials.h"

namespace avAnimation
{

struct AnimationManagerFinder : public osg::NodeVisitor
{
    osg::ref_ptr<osgAnimation::BasicAnimationManager> _am;
    osg::ref_ptr<osgAnimation::AnimationManagerBase>  _bm;

    AnimationManagerFinder() : osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN) {}
    void apply(osg::Node& node) {
        if (_am.valid())
            return;

        if (node.getUpdateCallback()) {
            osgAnimation::AnimationManagerBase* b = dynamic_cast<osgAnimation::AnimationManagerBase*>(node.getUpdateCallback());
            if (b) {
                _am = new osgAnimation::BasicAnimationManager(*b);
				_bm = b; 
				return;

            }
        }
        traverse(node);
    }
};


struct RigTransformHardware : public osgAnimation::RigTransformHardware
{

    void operator()(osgAnimation::RigGeometry& geom)
    {
        if (_needInit)
            if (!init(geom))
                return;
        computeMatrixPaletteUniform(geom.getMatrixFromSkeletonToGeometry(), geom.getInvMatrixFromSkeletonToGeometry());
    }

    bool init(osgAnimation::RigGeometry& geom)
    {
        osg::Vec3Array* pos = dynamic_cast<osg::Vec3Array*>(geom.getVertexArray());
        if (!pos) {
            osg::notify(osg::WARN) << "RigTransformHardware no vertex array in the geometry " << geom.getName() << std::endl;
            return false;
        }

        if (!geom.getSkeleton()) {
            osg::notify(osg::WARN) << "RigTransformHardware no skeleting set in geometry " << geom.getName() << std::endl;
            return false;
        }
        else
            osg::notify(osg::WARN) << "RigTransformHardware skeleton address " << reinterpret_cast<uint32_t>( geom.getSkeleton()) << std::endl;

        osgAnimation::BoneMapVisitor mapVisitor;
        geom.getSkeleton()->accept(mapVisitor);
        osgAnimation::BoneMap bm = mapVisitor.getBoneMap();

        if (!createPalette(pos->size(),bm, geom.getVertexInfluenceSet().getVertexToBoneList()))
            return false;

        int attribIndex = 11;
        int nbAttribs = getNumVertexAttrib();

        std::stringstream ss;
        ss << "#define MAX_MATRIX  " << getMatrixPaletteUniform()->getNumElements();
        
        osg::ref_ptr<osg::Program> program = creators::createProgram("skinning",ss.str()).program; 
        program->setName("HardwareSkinning");

        for (int i = 0; i < nbAttribs; i++)
        {
            std::stringstream ss;
            ss << "boneWeight" << i;
            program->addBindAttribLocation(ss.str(), attribIndex + i);

            osg::notify(osg::INFO) << "set vertex attrib " << ss.str() << std::endl;
        }

        for (int i = 0; i < nbAttribs; i++)
        {
            std::stringstream ss;
            ss << "boneWeight" << i;
            geom.setVertexAttribArray(attribIndex + i, getVertexAttrib(i));
        }
        
        //avCore::Database::GetInstance()->LoadTexture();

        geom.getOrCreateStateSet()->addUniform(getMatrixPaletteUniform());
        geom.getOrCreateStateSet()->addUniform(new osg::Uniform("nbBonesPerVertex", getNumBonesPerVertex()));
        geom.getOrCreateStateSet()->setAttributeAndModes(program.get());

        _needInit = false;
        return true;
    }

};

struct SkeletonFinder : public osg::NodeVisitor
{
    osg::ref_ptr<osgAnimation::Skeleton> _skel;
    SkeletonFinder() : osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN) {}

    void apply(osg::Node& node) {
#if 0
        if (_skel.valid())
            return;
#endif

        osgAnimation::Skeleton* b = dynamic_cast<osgAnimation::Skeleton*>(&node);
        if (b) {
            _skel = b;
            osg::notify(osg::WARN) << "SkeletonFinder address " << reinterpret_cast<uint32_t>( b ) << std::endl;
            //return;
        }

        traverse(node);
    }
};

struct SetupRigGeometry : public osg::NodeVisitor
{
    bool _hardware;
    osg::ref_ptr<osgAnimation::Skeleton> _skel;

    SetupRigGeometry( bool hardware = true) : osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN), _hardware(hardware) {}
    
    SetupRigGeometry( bool hardware, osg::Node& node ) : osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN), _hardware(hardware) 
    {
         SkeletonFinder sf;
         node.accept(sf);
         _skel = sf._skel;

         node.accept(*this);
    }

    void apply(osg::Geode& geode)
    {
        for (unsigned int i = 0; i < geode.getNumDrawables(); i++)
            apply(*geode.getDrawable(i));
    }

    void apply(osg::Drawable& geom)
    {
        if (_hardware) {
            osgAnimation::RigGeometry* rig = dynamic_cast<osgAnimation::RigGeometry*>(&geom);
            if (rig)
            {
                rig->setRigTransformImplementation(new avAnimation::RigTransformHardware);

                if(_skel)
                {
                    //rig->setSkeleton(_skel.get());
                    //rig->computeMatrixFromRootSkeleton();

                }


            }
        }
    }
};

class AnimtkViewerModelController 
{
public:
	typedef std::vector<std::string>        AnimationMapVector;
	typedef std::map   <std::string,double> AnimationDurationMap;

	static AnimtkViewerModelController& instance() 
	{
		static AnimtkViewerModelController avmc;
		return avmc;
	}

	static bool setModel(osgAnimation::BasicAnimationManager* model) 
	{
		AnimtkViewerModelController& self = instance();
		self._model = model;
		for (osgAnimation::AnimationList::const_iterator it = self._model->getAnimationList().begin(); it != self._model->getAnimationList().end(); it++)
            self._map[(*it)->getName() + "_base"] = *it;

		for(osgAnimation::AnimationMap::iterator it = self._map.begin(); it != self._map.end(); it++)
		{
            self._amv.push_back(it->first);
            it->second.get()->computeDuration();
            self._amd.insert(std::make_pair(it->first,it->second.get()->getDuration()));
        }
        
        //self._default = new osgAnimation::ActionStripAnimation(self._map[self._amv.back()].get(),0.0,0.0);
        //self._default->setLoop(0); // means forever

		return true;
	}

    static bool addAnimation(osg::Node* anim_container ) 
    {
        osgAnimation::BasicAnimationManager* model = dynamic_cast<osgAnimation::BasicAnimationManager*>(anim_container->getUpdateCallback());
        osgAnimation::AnimationMap local_map;

        if(!model) 
        {
            osg::notify(osg::FATAL) << "Did not find AnimationManagerBase updateCallback needed to animate elements" << std::endl;
            return false;
        }
        
        AnimtkViewerModelController& self = instance();
        
        for (osgAnimation::AnimationList::const_iterator it = model->getAnimationList().begin(); it != model->getAnimationList().end(); it++)
        {
            self._map[(*it)->getName() + "_" + anim_container->getName() ] = *it;
            local_map[(*it)->getName() + "_" + anim_container->getName() ] = *it;
            self._model->registerAnimation(*it);
        }

        for(osgAnimation::AnimationMap::iterator it = local_map.begin(); it != local_map.end(); it++)
        {
            self._amv.push_back(it->first);
            it->second.get()->computeDuration();
            self._amd.insert(std::make_pair(it->first,it->second.get()->getDuration()));
        }

        //self._default = new osgAnimation::ActionStripAnimation(self._map[self._amv.back()].get(),0.0,0.0);
        //self._default->setLoop(0); // means forever

        return true;
    }

	bool list() 
	{
		std::cout << "Animation List:" << std::endl;
		for(osgAnimation::AnimationMap::iterator it = _map.begin(); it != _map.end(); it++)
			std::cout << it->first << std::endl;
		return true;
	}

    osgAnimation::Animation* current()
    {
        return _map[_amv[_focus]].get();
    }

    void setPlayMode (osgAnimation::Animation::PlayMode mode)
    {
          if(_focus < _amv.size()) 
          {
              std::cout << "Play " << _amv[_focus] << std::endl;
              _map[_amv[_focus]].get()->setPlayMode(mode);
          }
    }
    
    void setDurationRatio (double ratio)
    {
        for(osgAnimation::AnimationMap::iterator it = _map.begin(); it != _map.end(); it++)
        {
            const std::string& name = it->first;
            it->second.get()->setDuration(_amd[name] * ratio);
        }
    }

	bool play() 
	{
		if(_focus < _amv.size()) 
		{
			std::cout << "Play " << _amv[_focus] << std::endl;
			_model->playAnimation(_map[_amv[_focus]].get());
			return true;
		}

		return false;
	}

	bool stop() 
	{
		if(_focus < _amv.size()) 
		{
			std::cout << "Stop " << _amv[_focus] << std::endl;
			_model->stopAnimation(_map[_amv[_focus]].get());
			return true;
		}
		return false;
	}    
	
	bool stopPrev() 
	{
		if(_prev_focus < _amv.size()) 
		{
			std::cout << "Stop " << _amv[_prev_focus] << std::endl;
			_model->stopAnimation(_map[_amv[_prev_focus]].get());
			return true;
		}
		return false;
	} 

	bool next() 
	{
		_prev_focus = _focus;
		_focus = (_focus + 1) % _map.size();
		std::cout << "Current now is " << _amv[_focus] << std::endl;
		return true;
	}

	bool previous() 
	{
		_prev_focus = _focus;
		_focus = (_map.size() + _focus - 1) % _map.size();
		std::cout << "Current now is " << _amv[_focus] << std::endl;
		return true;
	}

	bool playByName(const std::string& name) 
	{
		for(unsigned int i = 0; i < _amv.size(); i++) if(_amv[i] == name) _focus = i;
		_model->playAnimation(_map[name].get());
		return true;
	}

	const std::string& getCurrentAnimationName() const 
	{
		return _amv[_focus];
	}

	const AnimationMapVector& getAnimationMap() const 
	{
		return _amv;
	}

    bool playing()
    {
        return _model->isPlaying(_map[_amv[_focus]].get());
    }

    const osgAnimation::BasicAnimationManager* manager() const
    {
        return  _model;
    }


private:
	osg::ref_ptr<osgAnimation::BasicAnimationManager> _model;
	osgAnimation::AnimationMap                        _map;
	AnimationMapVector                                _amv;
    AnimationDurationMap                              _amd;
	unsigned int                                      _focus;
    unsigned int                                      _prev_focus;
    osg::ref_ptr<osgAnimation::ActionStripAnimation>  _default;

	AnimtkViewerModelController():
	_model(0),
		_focus(0) {}
};



class AnimationController 
{
public:
    typedef std::vector<std::string>        AnimationMapVector;
    typedef std::map   <std::string,double> AnimationDurationMap;

    bool setModel(osgAnimation::BasicAnimationManager* model) 
    {
        AnimationController& self = *this;
        self._model = model;
        for (osgAnimation::AnimationList::const_iterator it = self._model->getAnimationList().begin(); it != self._model->getAnimationList().end(); it++)
            self._map[(*it)->getName() + "_base"] = *it;

        for(osgAnimation::AnimationMap::iterator it = self._map.begin(); it != self._map.end(); it++)
        {
            self._amv.push_back(it->first);
            it->second.get()->computeDuration();
            self._amd.insert(std::make_pair(it->first,it->second.get()->getDuration()));
        }

        return true;
    }

    bool addAnimation(osg::Node* anim_container ) 
    {
        osgAnimation::BasicAnimationManager* model = dynamic_cast<osgAnimation::BasicAnimationManager*>(anim_container->getUpdateCallback());
        osgAnimation::AnimationMap local_map;

        if(!model) 
        {
            osg::notify(osg::FATAL) << "Did not find AnimationManagerBase updateCallback needed to animate elements" << std::endl;
            return false;
        }

        AnimationController& self = *this;

        for (osgAnimation::AnimationList::const_iterator it = model->getAnimationList().begin(); it != model->getAnimationList().end(); it++)
        {
            self._map[(*it)->getName() + "_" + anim_container->getName() ] = *it;
            local_map[(*it)->getName() + "_" + anim_container->getName() ] = *it;
            self._model->registerAnimation(*it);
        }

        for(osgAnimation::AnimationMap::iterator it = local_map.begin(); it != local_map.end(); it++)
        {
            self._amv.push_back(it->first);
            it->second.get()->computeDuration();
            self._amd.insert(std::make_pair(it->first,it->second.get()->getDuration()));
        }

        //self._default = new osgAnimation::ActionStripAnimation(self._map[self._amv.back()].get(),0.0,0.0);
        //self._default->setLoop(0); // means forever

        return true;
    }

    bool list() 
    {
        std::cout << "Animation List:" << std::endl;
        for(osgAnimation::AnimationMap::iterator it = _map.begin(); it != _map.end(); it++)
            std::cout << it->first << std::endl;
        return true;
    }

    osgAnimation::Animation* current()
    {
        return _map[_amv[_focus]].get();
    }

    void setPlayMode (osgAnimation::Animation::PlayMode mode)
    {
        if(_focus < _amv.size()) 
        {
            std::cout << "Play " << _amv[_focus] << std::endl;
            _map[_amv[_focus]].get()->setPlayMode(mode);
        }
    }

    void setDurationRatio (double ratio)
    {
        for(osgAnimation::AnimationMap::iterator it = _map.begin(); it != _map.end(); it++)
        {
            const std::string& name = it->first;
            it->second.get()->setDuration(_amd[name] * ratio);
        }
    }

    bool play() 
    {
        if(_focus < _amv.size()) 
        {
            std::cout << "Play " << _amv[_focus] << std::endl;
            _model->playAnimation(_map[_amv[_focus]].get());
            return true;
        }

        return false;
    }

    bool stop() 
    {
        if(_focus < _amv.size()) 
        {
            std::cout << "Stop " << _amv[_focus] << std::endl;
            _model->stopAnimation(_map[_amv[_focus]].get());
            return true;
        }
        return false;
    }    

    bool stopPrev() 
    {
        if(_prev_focus < _amv.size()) 
        {
            std::cout << "Stop " << _amv[_prev_focus] << std::endl;
            _model->stopAnimation(_map[_amv[_prev_focus]].get());
            return true;
        }
        return false;
    } 

    bool next() 
    {
        _prev_focus = _focus;
        _focus = (_focus + 1) % _map.size();
        std::cout << "Current now is " << _amv[_focus] << std::endl;
        return true;
    }

    bool previous() 
    {
        _prev_focus = _focus;
        _focus = (_map.size() + _focus - 1) % _map.size();
        std::cout << "Current now is " << _amv[_focus] << std::endl;
        return true;
    }

    bool playByName(const std::string& name) 
    {
        for(unsigned int i = 0; i < _amv.size(); i++)
            if(_amv[i] == name) _focus = i;
        
        _model->playAnimation(_map[name].get());
        return true;
    }

    const std::string& getCurrentAnimationName() const 
    {
        return _amv[_focus];
    }

    const AnimationMapVector& getAnimationMap() const 
    {
        return _amv;
    }

    bool playing()
    {
        return _model->isPlaying(_map[_amv[_focus]].get());
    }

    const osgAnimation::BasicAnimationManager* manager() const
    {
        return  _model;
    }


private:
    osg::ref_ptr<osgAnimation::BasicAnimationManager> _model;
    osgAnimation::AnimationMap                        _map;
    AnimationMapVector                                _amv;
    AnimationDurationMap                              _amd;
    unsigned int                                      _focus;
    unsigned int                                      _prev_focus;
    osg::ref_ptr<osgAnimation::ActionStripAnimation>  _default;

    AnimationController():
    _model(0),
        _focus(0) {}
};


}