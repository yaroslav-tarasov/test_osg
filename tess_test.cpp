#include "stdafx.h"

#include "creators.h"

namespace circles
{
    static const char vs[] = STRINGIFY ( 

        void main(void)
    {
        // gl_Position = gl_Vertex;
        gl_Position     = ftransform();//gl_ModelViewProjectionMatrix *  gl_Vertex; 
    }
    );

    static const char fs[] = STRINGIFY ( 
        void main(void)
    {
        vec2 pos = mod(gl_FragCoord.xy, vec2(50.0)) - vec2(25.0);
        float dist_squared = dot(pos, pos);

        gl_FragColor = (dist_squared < 400.0) 
            ? vec4(.90, .90, .90, 1.0)
            : vec4(.20, .20, .40, 1.0);
    }
    );

}

struct FaceCollector
{
    void operator()( const osg::Vec3& v1, const osg::Vec3& v2, const osg::Vec3& v3, bool )
    {
        std::cout << "Face vertices: " << v1 << "; " << v2 << "; " << v3 << std::endl;
    }
};

class AnimateCallback: public osg::Uniform::Callback
{

public:
    enum callback_t {UPDATE_PROJECT,UPDATE_VIEW};
    AnimateCallback(osgViewer::Viewer* v,callback_t type):viewer_(v),type_(type){};
    virtual void operator() ( osg::Uniform* uniform, osg::NodeVisitor* nv )
    {
        bool ret;
        osg::Matrixf m;
        if (type_ == UPDATE_PROJECT)
            m = viewer_->getCamera()->getProjectionMatrix();
        else
            m = viewer_->getCamera()->getViewMatrix();

        ret = uniform->set( m );
    }
    osgViewer::Viewer* viewer_;
    callback_t         type_;
};

int main_tess_test( int argc, char** argv )
{

    osgViewer::Viewer viewer;
    
    float Verts[] = {
        - 0.3 , -0.3 , 0 , 0.3,	// upleft
          0.3 , -0.3 , 0 , 0.6,	// upright
          0.3 ,  0.3 , 0 , 0.4,	// downright
        - 0.3 ,  0.3 , 0 , 0.1	// downleft
    };

#if 1
    osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array;
    vertices->push_back( osg::Vec3(0.0f, 0.0f, 0.0f) );
    vertices->push_back( osg::Vec3(0.0f, 0.0f, 1.0f) );
    vertices->push_back( osg::Vec3(1.0f, 0.0f, 0.0f) );
    vertices->push_back( osg::Vec3(1.0f, 0.0f, 1.5f) );
    vertices->push_back( osg::Vec3(2.0f, 0.0f, 0.0f) );
    vertices->push_back( osg::Vec3(2.0f, 0.0f, 1.0f) );
    vertices->push_back( osg::Vec3(3.0f, 0.0f, 0.0f) );
    vertices->push_back( osg::Vec3(3.0f, 0.0f, 1.5f) );
    vertices->push_back( osg::Vec3(4.0f, 0.0f, 0.0f) );
    vertices->push_back( osg::Vec3(4.0f, 0.0f, 1.0f) );
    
    osg::ref_ptr<osg::Vec3Array> normals = new osg::Vec3Array;
    normals->push_back( osg::Vec3(0.0f,-1.0f, 0.0f) );
    
    osg::ref_ptr<osg::Geometry> geom = new osg::Geometry;
    geom->setVertexArray( vertices.get() );
    geom->setNormalArray( normals.get() );
    geom->setNormalBinding( osg::Geometry::BIND_OVERALL );
    geom->addPrimitiveSet( new osg::DrawArrays(/*GL_QUAD_STRIP*/GL_PATCHES, 0, 9) );
    
    osg::TriangleFunctor<FaceCollector> functor;
    geom->accept( functor );
#else    
    osg::ref_ptr<osg::Geometry> geom = new osg::Geometry;

    int VertexCount = sizeof(Verts)/sizeof(float);
    osg::Vec4Array* vertices = new osg::Vec4Array();
    for(int i=0;i<VertexCount;i+=4){
        vertices->push_back(osg::Vec4(Verts[i],Verts[i+1],Verts[i+2],Verts[i+3]));
    }
    geom->setVertexArray(vertices);
    geom->addPrimitiveSet(new osg::DrawElementsUInt(osg::PrimitiveSet::PATCHES/*,IndexCount,Faces*/));
#endif



    effects::createProgramFromFiles(geom.get(),"shaders/tess.vs","shaders/tess.frag","shaders/tess.geo","shaders/tess_tri.tcs","shaders/tess_tri.tes");

    {
        osg::StateSet* stateset = geom->getOrCreateStateSet();
        auto uni_pu  = new osg::Uniform("projectionMatrix", osg::Matrixf(viewer.getCamera()->getProjectionMatrix()));
        auto uni_mvm = new osg::Uniform("modelViewMatrix" , osg::Matrixf(viewer.getCamera()->getViewMatrix()));
        stateset->setAttribute(new osg::PatchParameter(3));
        stateset->addUniform( uni_pu  );
        stateset->addUniform( uni_mvm ); 
        uni_pu->setUpdateCallback(new AnimateCallback(&viewer,AnimateCallback::UPDATE_PROJECT));
        uni_pu->setDataVariance(osg::Object::DYNAMIC);
        uni_mvm->setUpdateCallback(new AnimateCallback(&viewer,AnimateCallback::UPDATE_VIEW));
        uni_mvm->setDataVariance(osg::Object::DYNAMIC);
        stateset->setDataVariance(osg::Object::DYNAMIC);
    }

 

    osg::ref_ptr<osg::Geode> root = new osg::Geode;
    root->addDrawable( geom.get() );
    

    viewer.setSceneData( root.get() );
    return viewer.run();
}