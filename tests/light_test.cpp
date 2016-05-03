#include "stdafx.h"
#include "visitors/info_visitor.h"
#include "visitors/find_tex_visitor.h"
#include "creators.h"
#include "av/avTerrain/Terrain.h"

namespace
{
	const size_t size_x = 50;//1980;
	const size_t size_z = 50;//1200;

    void createDAIGeometry( osg::Geometry& geom, int nInstances=1 )
    {
        const float halfDimX( size_x / 2.0 );
        const float halfDimZ( size_z / 2.0);

        osg::Vec3Array* v = new osg::Vec3Array;
        v->resize( 4 );
        geom.setVertexArray( v );

        // Geometry for a single quad.

        (*v)[ 0 ] = osg::Vec3( -halfDimX, 0.,  -halfDimX );
        (*v)[ 1 ] = osg::Vec3( halfDimX, 0., -halfDimX );
        (*v)[ 2 ] = osg::Vec3( halfDimX, 0., halfDimZ );
        (*v)[ 3 ] = osg::Vec3( -halfDimX, 0., halfDimZ);
#if 0
		(*v)[ 0 ] = osg::Vec3(  0., 0., 0. );
		(*v)[ 1 ] = osg::Vec3( halfDimX*2.0f, 0., 0. );
		(*v)[ 2 ] = osg::Vec3( halfDimX*2.0f, 0., halfDimZ*2.0f );
		(*v)[ 3 ] = osg::Vec3( 0., 0., halfDimZ*2.0f );
#endif
        // create color array data (each corner of our triangle will have one color component)
        osg::Vec4Array* pColors = new osg::Vec4Array;
        pColors->push_back( osg::Vec4( 1.0f, 0.0f, 0.0f, 1.0f ) );
        pColors->push_back( osg::Vec4( 0.0f, 1.0f, 0.0f, 1.0f ) );
        pColors->push_back( osg::Vec4( 0.0f, 0.0f, 1.0f, 1.0f ) );
        pColors->push_back( osg::Vec4( 0.0f, 0.0f, 1.0f, 1.0f ) );
        geom.setColorArray( pColors );

        // make sure that our geometry is using one color per vertex
        geom.setColorBinding( osg::Geometry::BIND_PER_VERTEX );

        osg::Vec2Array* pTexCoords = new osg::Vec2Array( 4 );
        (*pTexCoords)[0].set( 0.0f, 0.0f );
        (*pTexCoords)[1].set( 1.0f, 0.0f );
        (*pTexCoords)[2].set( 1.0f, 1.0f );
        (*pTexCoords)[3].set( 0.0f, 1.0f );
        geom.setTexCoordArray( 0, pTexCoords );

		osg::Vec3 normal = osg::Vec3( 0.0f, 0.0f, 1.0f);
		normal.normalize();
		osg::Vec3Array* normals = new osg::Vec3Array;
		normals->push_back(normal);

		geom.setNormalArray(normals, osg::Array::BIND_OVERALL);
        // Use the DrawArraysInstanced PrimitiveSet and tell it to draw 1024 instances.
        geom.addPrimitiveSet( new osg::DrawArrays( GL_QUADS, 0, 4, nInstances ) );
    }


    osg::Geometry* createGeometry()
    {
        osg::Geometry* geom = new osg::Geometry;
        geom->setUseDisplayList( false );
        geom->setUseVertexBufferObjects( true );
        createDAIGeometry( *geom, 1*1 );

        return geom;
    }


	class UpdateCallback  : public osg::NodeCallback
	{

	public:
		UpdateCallback( )
		{}

		virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
		{	
			traverse(node,nv);
			
			osgUtil::CullVisitor * pCV = static_cast<osgUtil::CullVisitor *>(nv);
			avAssert(pCV);

			const osg::Matrixd mWorldToView = *pCV->getModelViewMatrix();

			osg::Matrix trMatrix = node->asTransform()->asMatrixTransform()->getMatrix();
		}


		osg::ref_ptr<osg::Uniform> uworldToLight;
	    osg::ref_ptr<osg::Uniform> ulightToWorld;

	};


}

int main_light_test( int argc, char** argv )
{
    osg::ArgumentParser arguments(&argc,argv);

    osgViewer::Viewer viewer(arguments);
	viewer.getCamera()->setClearColor(osg::Vec4(0,0,0,0));

    osgDB::getDataFilePathList().push_back(osgDB::getCurrentWorkingDirectory() + "\\data\\models");
    osgDB::getDataFilePathList().push_back(osgDB::getCurrentWorkingDirectory() + "\\data\\areas");
    osgDB::getDataFilePathList().push_back(osgDB::getCurrentWorkingDirectory() + "\\data\\materials");
    osgDB::getDataFilePathList().push_back(osgDB::getCurrentWorkingDirectory() + "\\data\\materials\\sky");
    osgDB::getDataFilePathList().push_back(osgDB::getCurrentWorkingDirectory() + "\\data\\materials\\lib");   

    osg::ref_ptr<osg::MatrixTransform> root = new osg::MatrixTransform;



    osg::Geode*		geodeLit = new osg::Geode();	
    osg::ref_ptr<osg::Geometry> geom = createGeometry();

    geodeLit->setCullingActive( false );   
    geodeLit->addDrawable( geom.get() );
    
	osg::StateSet * pStateSet = geodeLit->getOrCreateStateSet();
	osg::ref_ptr<osg::PositionAttitudeTransform> mt = new osg::PositionAttitudeTransform;
	mt->setPosition(osg::Vec3(0,14.5,1));
	mt->addChild(geodeLit);
    
	root->addChild(mt);
	// root->setNodeCallback(new UpdateCallback);


	const std::string name = "a_319";
	osg::Node* model_file = osgDB::readNodeFile("data/models/" + name + "/" + name + ".dae");
    root->addChild(model_file);
	model_file->getOrCreateStateSet()->setRenderBinDetails(2, "DepthSortedBin");

	osg::Matrix m;
	osg::Matrix mi;

	m.makeTranslate(osg::Vec3(10000,0,50));
	pStateSet->addUniform(new osg::Uniform("g_lightToWorld"  , m));

	mi = osg::Matrix::inverse(m);
	pStateSet->addUniform(new osg::Uniform("g_worldToLight"           , mi));
	pStateSet->addUniform(new osg::Uniform("g_lightCol"               , osg::Vec4(1,1,1,0.87)));
	pStateSet->addUniform(new osg::Uniform("g_scatteringCoefficient"  , 1.f));


    osg::ref_ptr<osg::Program> cLightProg = creators::createProgram("splight").program; 
    cLightProg->setName("LightShader");
    pStateSet->setAttributeAndModes(cLightProg.get());
	pStateSet->setRenderingHint( osg::StateSet::TRANSPARENT_BIN );
	
	pStateSet->setMode( GL_ALPHA_TEST, osg::StateAttribute::ON | osg::StateAttribute::PROTECTED );
	//pStateSet->setAttribute( new osg::AlphaFunc(osg::AlphaFunc::GREATER, 0.99f), osg::StateAttribute::ON );

	// setup blending
	osg::BlendFunc * pBlendFunc = new osg::BlendFunc(osg::BlendFunc::SRC_ALPHA, osg::BlendFunc::ONE_MINUS_SRC_ALPHA);
	pStateSet->setAttributeAndModes(pBlendFunc, osg::StateAttribute::ON);

	osg::BlendEquation* pBlendEquation = new osg::BlendEquation(osg::BlendEquation::FUNC_ADD);
	pStateSet->setAttributeAndModes(pBlendEquation,osg::StateAttribute::OVERRIDE|osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);
	
	//pStateSet->setRenderBinDetails(0, "DepthSortedBin");
	//pStateSet->setRenderBinDetails(1, "DepthSortedBin");
	//pStateSet->setNestRenderBins(false);

	viewer.apply(new osgViewer::SingleScreen(1));
    // Add some useful handlers to see stats, wireframe and onscreen help
    viewer.addEventHandler(new osgViewer::StatsHandler);
    viewer.addEventHandler(new osgGA::StateSetManipulator(root->getOrCreateStateSet()));
    viewer.addEventHandler(new osgViewer::HelpHandler);
    viewer.setSceneData( root.get() );
    return viewer.run();
}

AUTO_REG(main_light_test)