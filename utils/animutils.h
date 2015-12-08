#pragma once

#include <osgAnimation/AnimationManagerBase>
#include <osgAnimation/Bone>

#include <osgAnimation/ActionStripAnimation>
#include <osgAnimation/ActionBlendIn>
#include <osgAnimation/ActionBlendOut>
#include <osgAnimation/ActionAnimation>

namespace avAnimation
{

	struct AnimationManagerFinder : public osg::NodeVisitor
{
    osg::ref_ptr<osgAnimation::BasicAnimationManager> _am;
    AnimationManagerFinder() : osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN) {}
    void apply(osg::Node& node) {
        if (_am.valid())
            return;
        if (node.getUpdateCallback()) {
            osgAnimation::AnimationManagerBase* b = dynamic_cast<osgAnimation::AnimationManagerBase*>(node.getUpdateCallback());
            if (b) {
                _am = new osgAnimation::BasicAnimationManager(*b);
                return;
            }
        }
        traverse(node);
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
			self._map[(*it)->getName()] = *it;

		for(osgAnimation::AnimationMap::iterator it = self._map.begin(); it != self._map.end(); it++)
		{
            self._amv.push_back(it->first);
            it->second.get()->computeDuration();
            self._amd.insert(std::make_pair(it->first,it->second.get()->getDuration()));
        }
        
        self._default = new osgAnimation::ActionStripAnimation(self._map[self._amv.back()].get(),0.0,0.0);
        self._default->setLoop(0); // means forever

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
        if(_focus < _amv.size()) 
        {
            const std::string& name = _amv[_focus];
            std::cout << "Play " << name << std::endl;
            _map[name].get()->setDuration(_amd[name] * ratio);
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

	bool next() 
	{
		_focus = (_focus + 1) % _map.size();
		std::cout << "Current now is " << _amv[_focus] << std::endl;
		return true;
	}

	bool previous() 
	{
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
        return _model->isPlaying(_amv[_focus]);
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
    osg::ref_ptr<osgAnimation::ActionStripAnimation>  _default;

	AnimtkViewerModelController():
	_model(0),
		_focus(0) {}
};

}