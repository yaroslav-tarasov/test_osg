#pragma once



namespace avTerrain
{

	class Grass : public osg::Group
	{
	public:
		Grass ();
	private:
		osg::Node*			_create();
		osg::StateSet*      _buildStateSet(osg::Node* node);
	private:
		osg::ref_ptr<osg::Uniform>		unf_spacing_;
		osg::ref_ptr<osg::Uniform>		unfCC_;
		osg::ref_ptr<osg::Uniform>		unfLock_;
		osg::ref_ptr<osg::Uniform>		unfHeightAdjust_;
		osg::ref_ptr<osg::Uniform>		unfWindFactor_;
		osg::ref_ptr<osg::Uniform>		unfGrassStretch_;
		osg::ref_ptr<osg::Uniform>		unfHeightMap_;
	};

}
