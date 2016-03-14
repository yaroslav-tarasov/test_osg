#pragma once

namespace blender
{

	class Grass : public osg::Group
	{
	public:
		Grass ();
		void setWindFactor(float wf);
		void setGrassMapFactor(float val);
	private:
		osg::Node*			_create();
		void                _buildStateSet(osg::Node* node);
		osg::Geometry*		_createGeometry();
		bool				_createMap();

	private:
		osg::ref_ptr<osg::Uniform>		unfSpacing_;
		osg::ref_ptr<osg::Uniform>		unfInstances_;
		osg::ref_ptr<osg::Uniform>		unfLock_;
		osg::ref_ptr<osg::Uniform>		unfHeightAdjust_;
		osg::ref_ptr<osg::Uniform>		unfWindFactor_;
		osg::ref_ptr<osg::Uniform>		unfGrassStretch_;
		osg::ref_ptr<osg::Uniform>		unfHeightMap_;

		osg::ref_ptr<osg::Texture2D>    _map;
		osg::ref_ptr<osg::Geode>	    _geodeGrass;
		osg::ref_ptr<osg::Uniform>		unfGrassMapFactor_;

	private:
		float							_windFactor;
		float							_instances	;		// grid cells _instances*_instances
		float							_spacing		;			// spacing
		float							_heightAdjust	;
		float							_grassStretch   ;	

	private:
		unsigned _texWidth;
		unsigned _texHeight;
		unsigned _width;
		unsigned _height;
	};

}
