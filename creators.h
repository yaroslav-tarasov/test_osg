#pragma once


namespace creators 
{
    typedef std::array<osg::Node*, 6> nodes_array_t;

    osg::AnimationPath* createAnimationPath(const osg::Vec3& center,float radius,double looptime);
    osg::Node*    createBase(const osg::Vec3& center,float radius);
    nodes_array_t createMovingModel(const osg::Vec3& center, float radius);
    nodes_array_t createModel(bool overlay, osgSim::OverlayNode::OverlayTechnique technique);
    osg::Node*    loadAirplane();
    nodes_array_t loadAirplaneParts();
    osg::Node*    loadBMAirplane();

    class programsHolder_base{
    public:
        struct program_t
        {
            osg::ref_ptr<osg::Program> program;
        };
    };


    programsHolder_base::program_t   CreateProgram(std::string mat_name);

}

namespace effects
{
     void insertParticle(osg::Group* root,osg::Node* rootModel, const osg::Vec3& center, float radius);
     osg::Node* createLightSource( unsigned int num,
         const osg::Vec3& trans,
         const osg::Vec4& color );

     template<typename G>
     void createProgram(G* geom,std::string vs = "",std::string fs = "",std::string gs = "", std::string tcs = "", std::string tes = "" )
     {
         osg::StateSet* stateset = geom->getOrCreateStateSet();

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

enum {
    RENDER_BIN_SCENE                    =  0,
    RENDER_BIN_SKYFOG                   = -1, // global sky fog layer
    RENDER_BIN_CLOUDS                   = -2, 
};