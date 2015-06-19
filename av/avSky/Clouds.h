#ifndef __SV_CLOUDS_H__
#define __SV_CLOUDS_H__

#include <osg/Group>

namespace avSky
{
class Clouds : public osg::Group
{
public:
								Clouds();
	virtual bool				Initialize();
	virtual bool				FrameCall();

private:
	osg::ref_ptr<osg::Uniform>	_cCloudsParamsPtr;
	osg::ref_ptr<osg::Uniform>	_cCloudsMaskPtr;
	osg::ref_ptr<osg::Uniform>	_cCloudsColorPtr;
};
}

#endif // __SV_CLOUDS_H__
