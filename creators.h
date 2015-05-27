#pragma once

#include "materials.h"


namespace creators 
{
    typedef std::array<osg::Node*, 6> nodes_array_t;

    osg::AnimationPath* createAnimationPath(const osg::Vec3& center,float radius,double looptime);
    osg::Node*          createBase(const osg::Vec3& center,float radius);
    nodes_array_t       createMovingModel(const osg::Vec3& center, float radius);
    nodes_array_t       createModel(osg::ref_ptr<osg::LightSource>& ls,bool overlay, osgSim::OverlayNode::OverlayTechnique technique);
    osg::Node*          loadAirplane(std::string);
    nodes_array_t       loadAirplaneParts(std::string);
    osg::Node*          applyBM(osg::Node* model, std::string name,bool set_env_tex = false );
    osg::Node*          loadHelicopter();
	osg::Node*          createObject(std::string name, bool fclone=true);
    void                releaseObjectCache();

    const osg::Vec4 red_color   (100.0f, 0.0f, 0.0f, 100.0f);
    const osg::Vec4 blue_color  (0.0f, 0.0f, 100.0f, 100.0f);
    const osg::Vec4 green_color (0.0f, 100.0f, 0.0f, 100.0f);
    const osg::Vec4 white_color (100.0f, 100.0f, 100.0f, 100.0f);
    const osg::Vec4 black_color (0.0f,0.0f,0.0f,100.0f);
    const osg::Vec4 gray_color  (0.8f,0.8f,0.8f,100.0f);
}




namespace bi
{
    osg::ref_ptr<osgGA::GUIEventHandler>& getUpdater();
}


namespace effects
{
     void insertParticle(osg::Group* root,osg::Node* rootModel, const osg::Vec3& center, float radius);
     osg::Node* createLightSource( unsigned int num,
         const osg::Vec3& trans,
         const osg::Vec4& color );

     // template<typename G>
     inline  osg::Program* createProgram(osg::StateSet* stateset,std::string vs = "",std::string fs = "",std::string gs = "", std::string tcs = "", std::string tes = "" )
     {
         //osg::StateSet* stateset = geom->getOrCreateStateSet();

         osg::Program* program = new osg::Program;

         stateset->setAttribute(program);


         if (!vs.empty())
         {
             auto s_ = new osg::Shader(osg::Shader::VERTEX, vs);
             program->addShader( s_ );
         }

         if (!fs.empty())
         {
             auto s_ = new osg::Shader( osg::Shader::FRAGMENT, fs );
             program->addShader( s_ );
         }

         if (!gs.empty())
         {
             auto s_ = new osg::Shader( osg::Shader::GEOMETRY, gs );
             program->addShader( s_ );
         }

         if (!tcs.empty())
         {
             auto s_ = new osg::Shader( osg::Shader::TESSCONTROL, tcs );
             program->addShader( s_ );
         }

         if (!tes.empty())
         {
             auto s_ = new osg::Shader( osg::Shader::TESSEVALUATION, tes );
             program->addShader( s_ );
         }

         return program;
     }

     template<typename G>
     void createProgramFromFiles(G* geom,std::string vs = "",std::string fs = "",std::string gs = "", std::string tcs = "", std::string tes = "" )
     {
         osg::StateSet* stateset = geom->getOrCreateStateSet();

         osg::Program* program = new osg::Program;

         stateset->setAttribute(program);


         if (!vs.empty())
         {
             program->addShader( osg::Shader::readShaderFile(osg::Shader::VERTEX, osgDB::findDataFile(vs)) );
         }

         if (!fs.empty())
         {
             program->addShader( osg::Shader::readShaderFile(osg::Shader::FRAGMENT, osgDB::findDataFile(fs)) );
         }

         if (!gs.empty())
         {
             program->addShader( osg::Shader::readShaderFile(osg::Shader::GEOMETRY, osgDB::findDataFile(gs)) );
         }

         if (!tcs.empty())
         {
             program->addShader( osg::Shader::readShaderFile(osg::Shader::TESSCONTROL, osgDB::findDataFile(tcs)) );
         }

         if (!tes.empty())
         {
             program->addShader( osg::Shader::readShaderFile(osg::Shader::TESSEVALUATION, osgDB::findDataFile(tes)) );
         }
     }
}

namespace utils
{
    bool replace(std::string& str, const std::string& from, const std::string& to);
    void replaceAll(std::string& str, const std::string& from, const std::string& to);
    std::string format( const char * str );
    std::string format( std::string const & str );
}

enum render_order_t {
    RENDER_BIN_SCENE                    =  0,
    RENDER_BIN_SKYFOG                   = -1, // global sky fog layer
    RENDER_BIN_CLOUDS                   = -2, 
};

enum masks_t{
    NODE_STARFIELD_MASK                 = 0x2
};

namespace spark
{   
    typedef std::pair<osg::Node*, osgGA::GUIEventHandler*> spark_pair_t;  
    enum spark_t {EXPLOSION,FIRE,RAIN,SMOKE,TEST};
    void init();
    spark_pair_t create(spark_t effectType,osg::Transform* model=nullptr);
}

