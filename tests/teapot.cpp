#include "stdafx.h"
#include "teapot.h"

struct SpotRenderVertex
{
    cg::point_3f                    v;
    cg::point_3f                    from_l;
    cg::point_3f                    ldir;
    cg::colorf                      lcolor;
    cg::point_2f                    dist_falloff;
    cg::point_2f                    cone_falloff;
};
namespace {

char vertexShaderSource[] = 
    "#extension GL_ARB_gpu_shader5 : enable \n"
    "\n"
    "void main(void) \n"
    "{ \n"
    "\n"
    "    gl_Position =   gl_ModelViewProjectionMatrix * gl_Vertex;\n"
    "}\n";

char fragmentShaderSource[] = 
    "#extension GL_ARB_gpu_shader5 : enable \n"
    "\n"
    "uniform vec4 color;\n"
    "void main(void) \n"
    "{ \n"
    "    gl_FragColor = color; \n"
    "}\n";

}

osg::Geometry * _createGeometry()
{
    // dummy bounding box callback
    osg::Drawable::ComputeBoundingBoxCallback * pDummyBBCompute = new osg::Drawable::ComputeBoundingBoxCallback();

    // create OSG geode with 1 drawable node
    //setCullingActive(false);
    //setDataVariance(osg::Object::STATIC);

    // create tetrahedron around viewer (just to fill the whole volume)
    osg::Geometry * box_geometry = new osg::Geometry;
    box_geometry->setUseDisplayList(true);
    box_geometry->setDataVariance(osg::Object::STATIC);
    box_geometry->setComputeBoundingBoxCallback(pDummyBBCompute);

    return box_geometry;
}

void add_something(osg::Geometry* _geom )
{

    SpotRenderVertex  new_v;
    
    std::vector<SpotRenderVertex> vertices_;

    std::vector<SpotRenderVertex> new_spot(3 * 2, new_v);

    SpotRenderVertex & cur_v0 = new_spot[0];
    cur_v0.v = cg::point_3f(13, 13,  12);
    SpotRenderVertex & cur_v1 = new_spot[1];
    cur_v1.v = cg::point_3f(13, -13, -12);
    SpotRenderVertex & cur_v2 = new_spot[2];
    cur_v2.v = cg::point_3f(-13, -13, 0);

    SpotRenderVertex & cur_v3 = new_spot[3];
    cur_v3.v = cg::point_3f(13, 13,  12);
    SpotRenderVertex & cur_v4 = new_spot[4];
    cur_v4.v = cg::point_3f(-13, -13, 0);
    SpotRenderVertex & cur_v5 = new_spot[5];
    cur_v5.v = cg::point_3f(-13, 13, -12);

    // append them
    vertices_.insert(vertices_.end(), new_spot.begin(), new_spot.end());

    // create its' vertex
    osg::Vec3Array * paBoxPointsPos = new osg::Vec3Array;
    paBoxPointsPos->resize(vertices_.size());

    //from_l_->resize(vertices_.size());
    //ldir_->resize(vertices_.size());
    //dist_falloff_->resize(vertices_.size());
    //cone_falloff_->resize(vertices_.size());
    //lcolor_->resize(vertices_.size());

    for (unsigned i = 0; i < vertices_.size(); ++i)
    {
        SpotRenderVertex & cur_v = vertices_[i];
        paBoxPointsPos->at(i).set(cur_v.v.x, cur_v.v.y, cur_v.v.z);
        //from_l_->at(i).set( cur_v.from_l.x, cur_v.from_l.y,cur_v.from_l.z );
        //ldir_->at(i).set( cur_v.ldir.x, cur_v.ldir.y, cur_v.ldir.z );
        //dist_falloff_->at(i).set( cur_v.dist_falloff.x, cur_v.dist_falloff.y  );
        //cone_falloff_->at(i).set( cur_v.cone_falloff.x, cur_v.cone_falloff.y );
        //lcolor_->at(i).set(  cur_v.lcolor.r, cur_v.lcolor.g, cur_v.lcolor.b  );
    }

    // set vertex array
    paBoxPointsPos->setDataVariance(osg::Object::DYNAMIC);
    _geom->setVertexArray(paBoxPointsPos);

    _geom->addPrimitiveSet( new osg::DrawArrays( GL_TRIANGLES, 0, vertices_.size() ) );

    osg::StateSet* stateset = _geom->getOrCreateStateSet();

    osg::Program* program = new osg::Program;

    osg::Shader* vertex_shader = new osg::Shader(osg::Shader::VERTEX, vertexShaderSource);
    //program->addShader(vertex_shader);

    osg::Shader* fragment_shader = new osg::Shader(osg::Shader::FRAGMENT, fragmentShaderSource);
    program->addShader(fragment_shader);

    program->setName("LightMap");
    stateset->setAttribute(program);
};



int main_teapot( int argc, char** argv )
{
    osg::ArgumentParser arguments(&argc,argv);
    
    osg::setNotifyLevel( osg::INFO ); 

	osg::ref_ptr<osg::Group> root= new osg::Group;
	osg::ref_ptr<osg::Geode> teapot = new osg::Geode;
	teapot->addDrawable( new TeapotDrawable(1.0f) );
	//root->addChild(teapot);
    
    osg::CopyOp  copyop =  osg::CopyOp::DEEP_COPY_ALL
        & ~osg::CopyOp::DEEP_COPY_PRIMITIVES 
        & ~osg::CopyOp::DEEP_COPY_ARRAYS
        & ~osg::CopyOp::DEEP_COPY_IMAGES
        & ~osg::CopyOp::DEEP_COPY_TEXTURES  
        & ~osg::CopyOp::DEEP_COPY_STATEATTRIBUTES
#if 0
        & ~osg::CopyOp::DEEP_COPY_STATESETS
        & ~osg::CopyOp::DEEP_COPY_UNIFORMS
#endif
        & ~osg::CopyOp::DEEP_COPY_DRAWABLES
        //& ~osg::CopyOp::DEEP_COPY_CALLBACKS  // wo anim
        ;

    osg::ref_ptr<osg::Geode> smthg = new osg::Geode;
    auto geom = _createGeometry();
    add_something(geom);
    smthg->addDrawable(geom);
    
    osg::Matrix mat; 
    mat.setTrans(osg::Vec3(100,100, 0));

    osg::MatrixTransform* first = new osg::MatrixTransform(mat);
    osg::MatrixTransform* second = new osg::MatrixTransform;
    
    osg::PositionAttitudeTransform* pat = new  osg::PositionAttitudeTransform;
    
    pat->getOrCreateStateSet()->addUniform(new osg::Uniform("color", osg::Vec4(1.0,1.0,0.0,1.0)));
    pat->addChild(smthg);
    osg::Node* pat2 = copyop(pat);

    auto uni = pat2->getStateSet()->getUniform("color");
    uni->set(osg::Vec4(1.0,0.0,0.0,1.0));

    first->addChild(pat);
    second->addChild(pat2/*copyop(smthg)*//* osg::clone(smthg.get(),osg::CopyOp::DEEP_COPY_ALL)*/);

    root->addChild(first);
    root->addChild(second);


    // second->getOrCreateStateSet()->addUniform(new osg::Uniform("color", osg::Vec4(1.0,0.0,0.0,1.0)));

    osgViewer::Viewer viewer(arguments);

    viewer.getCamera()->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
    //viewer.getCamera()->setProjectionMatrix(osg::Matrixd::identity());
    //viewer.getCamera()->setViewMatrix(osg::Matrixd::scale(1,1,-1)); 


    viewer.getCamera()->setCullingMode( osg::CullSettings::NO_CULLING );
	viewer.apply(new osgViewer::SingleScreen(1));

    viewer.setSceneData( root.get() );
    return viewer.run();
}

 AUTO_REG(main_teapot)