#include "empty_scene.h"

namespace creators
{

osg::Node* createBase(const osg::Vec3& center,float radius)
{

	int numTilesX = 10;
	int numTilesY = 10;

	float width = 2*radius;
	float height = 2*radius;

	osg::Vec3 v000(center - osg::Vec3(width*0.5f,height*0.5f,0.0f));
	osg::Vec3 dx(osg::Vec3(width/((float)numTilesX),0.0,0.0f));
	osg::Vec3 dy(osg::Vec3(0.0f,height/((float)numTilesY),0.0f));

	// fill in vertices for grid, note numTilesX+1 * numTilesY+1...
	osg::Vec3Array* coords = new osg::Vec3Array;
	int iy;
	for(iy=0;iy<=numTilesY;++iy)
	{
		for(int ix=0;ix<=numTilesX;++ix)
		{
			coords->push_back(v000+dx*(float)ix+dy*(float)iy);
		}
	}

	//Just two colours - black and white.
	osg::Vec4Array* colors = new osg::Vec4Array;
	colors->push_back(osg::Vec4(1.0f,1.0f,1.0f,1.0f)); // white
	colors->push_back(osg::Vec4(0.0f,0.0f,0.0f,1.0f)); // black

	osg::ref_ptr<osg::DrawElementsUShort> whitePrimitives = new osg::DrawElementsUShort(GL_QUADS);
	osg::ref_ptr<osg::DrawElementsUShort> blackPrimitives = new osg::DrawElementsUShort(GL_QUADS);

	int numIndicesPerRow=numTilesX+1;
	for(iy=0;iy<numTilesY;++iy)
	{
		for(int ix=0;ix<numTilesX;++ix)
		{
			osg::DrawElementsUShort* primitives = ((iy+ix)%2==0) ? whitePrimitives.get() : blackPrimitives.get();
			primitives->push_back(ix    +(iy+1)*numIndicesPerRow);
			primitives->push_back(ix    +iy*numIndicesPerRow);
			primitives->push_back((ix+1)+iy*numIndicesPerRow);
			primitives->push_back((ix+1)+(iy+1)*numIndicesPerRow);
		}
	}

	// set up a single normal
	osg::Vec3Array* normals = new osg::Vec3Array;
	normals->push_back(osg::Vec3(0.0f,0.0f,1.0f));

	osg::Geometry* geom = new osg::Geometry;
	geom->setVertexArray(coords);

	geom->setColorArray(colors, osg::Array::BIND_PER_PRIMITIVE_SET);

	geom->setNormalArray(normals, osg::Array::BIND_OVERALL);

	geom->addPrimitiveSet(whitePrimitives.get());
	geom->addPrimitiveSet(blackPrimitives.get());

	osg::Geode* geode = new osg::Geode;
	geode->addDrawable(geom);
	//
	//osg::MatrixTransform* positioned = new osg::MatrixTransform;
	//positioned->addChild(geode);
	// 
#if defined(DEVELOP_SHADOWS)
	{
		effects::createProgram(geode->getOrCreateStateSet(),base_model::vs,base_model::fs) ;
		osg::StateAttribute::GLModeValue value = osg::StateAttribute::ON|osg::StateAttribute::OVERRIDE;
		geode->getOrCreateStateSet()->addUniform( new osg::Uniform("ShadowSplit0", 4) );
		geode->getOrCreateStateSet()->setTextureAttributeAndModes( 4, GetShadowMap()->getTexture(), value ); 
	}
#endif

	return geode;
}


}