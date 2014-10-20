/* -*-c++-*- OpenSceneGraph Cookbook
 * Chapter 6 Recipe 1
 * Author: Wang Rui <wangray84 at gmail dot com>
*/

#include "stdafx.h"
#include "creators.h"
#include "CommonFunctions"

static const char* vertSource = STRINGIFY ( 
    attribute vec3 tangent;
    attribute vec3 binormal;
    varying   vec3 lightDir;
    void main()
    {
        vec3 normal = normalize(gl_NormalMatrix * gl_Normal);
        mat3 rotation = mat3(tangent, binormal, normal);
        vec4 vertexInEye = gl_ModelViewMatrix * gl_Vertex;
        lightDir = vec3(gl_LightSource[0].position.xyz - vertexInEye.xyz);
        lightDir = normalize(rotation * normalize(lightDir));
        gl_Position = ftransform();
        gl_TexCoord[0] = gl_MultiTexCoord1;
    }
    );

static const char* fragSource = STRINGIFY ( 
    uniform sampler2D colorTex;
    uniform sampler2D normalTex;
    varying vec3 lightDir;
    void main (void)
    {
        vec4 base = texture2D(colorTex, gl_TexCoord[0].xy);
        vec3 bump = texture2D(normalTex, gl_TexCoord[0].xy).xyz;
        bump = normalize(bump * 2.0 - 1.0);
    
        float lambert = max(dot(bump, lightDir), 0.0);
        if (lambert > 0.0)
        {
            gl_FragColor = base * gl_LightSource[0].diffuse * lambert;
            gl_FragColor += gl_LightSource[0].specular * pow(lambert, 2.0);
        }
        gl_FragColor += gl_LightSource[0].ambient;
    }
    );

class ComputeTangentVisitor : public osg::NodeVisitor
{
public:
    void apply( osg::Node& node ) { traverse(node); }
    
    void apply( osg::Geode& node )
    {
        for ( unsigned int i=0; i<node.getNumDrawables(); ++i )
        {
            osg::Geometry* geom = dynamic_cast<osg::Geometry*>( node.getDrawable(i) );
            if ( geom ) generateTangentArray( geom );
        }
        traverse( node );
    }
    
    void generateTangentArray( osg::Geometry* geom )
    {
        osg::ref_ptr<osgUtil::TangentSpaceGenerator> tsg = new osgUtil::TangentSpaceGenerator;
        tsg->generate( geom, 1 );
        geom->setVertexAttribArray( 6, static_cast<osg::Array*>(tsg->getTangentArray()) );
        geom->setVertexAttribBinding( 6, osg::Geometry::BIND_PER_VERTEX );
        geom->setVertexAttribArray( 7, static_cast<osg::Array*>(tsg->getTangentArray()) );
        geom->setVertexAttribBinding( 7, osg::Geometry::BIND_PER_VERTEX );
    }
};

osg::Geometry* createOctahedron( const osg::Vec3& center )
{
    osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array(6);
    (*vertices)[0].set( 0.0f, 0.0f, 1.0f); (*vertices)[0] += center;
    (*vertices)[1].set(-0.5f,-0.5f, 0.0f); (*vertices)[0] += center;
    (*vertices)[2].set( 0.5f,-0.5f, 0.0f); (*vertices)[0] += center;
    (*vertices)[3].set( 0.5f, 0.5f, 0.0f); (*vertices)[0] += center;
    (*vertices)[4].set(-0.5f, 0.5f, 0.0f); (*vertices)[0] += center;
    (*vertices)[5].set( 0.0f, 0.0f,-1.0f); (*vertices)[0] += center;

    osg::ref_ptr<osg::DrawElementsUInt> indices = new osg::DrawElementsUInt(GL_TRIANGLES, 24);
    (*indices)[0] = 0; (*indices)[1] = 1; (*indices)[2] = 2;
    (*indices)[3] = 0; (*indices)[4] = 2; (*indices)[5] = 3;
    (*indices)[6] = 0; (*indices)[7] = 3; (*indices)[8] = 4;
    (*indices)[9] = 0; (*indices)[10]= 4; (*indices)[11]= 1;
    (*indices)[12]= 5; (*indices)[13]= 2; (*indices)[14]= 1;
    (*indices)[15]= 5; (*indices)[16]= 3; (*indices)[17]= 2;
    (*indices)[18]= 5; (*indices)[19]= 4; (*indices)[20]= 3;
    (*indices)[21]= 5; (*indices)[22]= 1; (*indices)[23]= 4;
    
    osg::ref_ptr<osg::Vec2Array> texcoords = new osg::Vec2Array;
    texcoords->push_back( osg::Vec2(0.0f, 0.0f) );
    texcoords->push_back( osg::Vec2(0.0f, 1.0f) );
    texcoords->push_back( osg::Vec2(1.0f, 1.0f) );
    texcoords->push_back( osg::Vec2(1.0f, 0.0f) );
    texcoords->push_back( osg::Vec2(0.5f, 1.0f) );
    texcoords->push_back( osg::Vec2(1.0f, 0.5f) );

    osg::ref_ptr<osg::Geometry> geom = new osg::Geometry;
    geom->setVertexArray( vertices.get() );
    geom->addPrimitiveSet( indices.get() );
    geom->setTexCoordArray( 0, texcoords.get() );

    // osgUtil::SmoothingVisitor::smooth( *geom );
    return geom.release();
}

osg::Node* CreateEarth()
{
    const double r_earth = 63.137;

    osg::ShapeDrawable *earth_sd = new osg::ShapeDrawable;
    osg::Sphere* earth_sphere = new osg::Sphere;
    earth_sphere->setName("EarthSphere");
    earth_sphere->setRadius(r_earth);
    earth_sd->setShape(earth_sphere);
    earth_sd->setColor(osg::Vec4(0, 0, 1.0, 1.0));

    osg::Geode* earth_geode = new osg::Geode;
    earth_geode->setName("EarthGeode");
    earth_geode->addDrawable(earth_sd);
     
    osg::ref_ptr<osg::Geode> oct = new osg::Geode;
    oct->addDrawable( createOctahedron(osg::Vec3()) );

    osg::Group* root = new osg::Group;
    //root->addChild(earth_geode);
    //root->addChild(osgDB::readNodeFile("cessna2.osgt")); 
    root->addChild(oct);
    return root ;
}

int main_bump_map( int argc, char** argv )
{
    osg::ArgumentParser arguments( &argc, argv );
    osg::ref_ptr<osg::Node> scene = osgDB::readNodeFiles( arguments );
    if ( !scene ) scene = creators::loadAirplane();// CreateEarth(); //osgDB::readNodeFile("skydome.osgt");//  //osgDB::readNodeFile("spaceship.osgt"); // 
    
    ComputeTangentVisitor ctv;
    ctv.setTraversalMode( osg::NodeVisitor::TRAVERSE_ALL_CHILDREN );
    scene->accept( ctv );
    
    osg::ref_ptr<osg::Program> program = new osg::Program;
    program->addShader( new osg::Shader(osg::Shader::VERTEX, vertSource) );
    program->addShader( new osg::Shader(osg::Shader::FRAGMENT, fragSource) );
    program->addBindAttribLocation( "tangent", 6 );
    program->addBindAttribLocation( "binormal", 7 );
                                                         
    //osg::ref_ptr<osg::Texture2D> colorTex = new osg::Texture2D;
    //colorTex->setImage( osgDB::readImageFile("Images/whitemetal_diffuse.jpg") );
    //
    //osg::ref_ptr<osg::Texture2D> normalTex = new osg::Texture2D;
    //normalTex->setImage( osgDB::readImageFile("Images/whitemetal_normal.jpg") );

    osg::ref_ptr<osg::Texture2D> colorTex = new osg::Texture2D;
    colorTex->setImage( osgDB::readImageFile("a_319_airfrance.png") );
    
    osg::ref_ptr<osg::Texture2D> normalTex = new osg::Texture2D;
    normalTex->setImage( osgDB::readImageFile("a_319_n.png") );  

    osg::StateSet* stateset = scene->getOrCreateStateSet();
    stateset->addUniform( new osg::Uniform("colorTex", 0) );
    stateset->addUniform( new osg::Uniform("normalTex", 1) );
    stateset->setAttributeAndModes( program.get() );

    osg::StateAttribute::GLModeValue value = osg::StateAttribute::ON|osg::StateAttribute::OVERRIDE;
    stateset->setTextureAttributeAndModes( 0, colorTex.get(), value );
    stateset->setTextureAttributeAndModes( 1, normalTex.get(), value );
    
    osgDB::writeNodeFile(*scene,"bump_mapping_test.osgt");
    osgViewer::Viewer viewer;
    viewer.setSceneData( scene.get() );
    return viewer.run();
}
