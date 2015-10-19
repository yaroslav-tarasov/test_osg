#include "stdafx.h"
#include "materials.h"
#include "av/shaders.h"
#include "av/avCore/Utils.h"

#include "visitors/ct_visitor.h"

//
//  ext
//
#include "pugixml.hpp"

namespace creators 
{
//class FogCallback: public osg::Uniform::Callback
//{
//public:
//    virtual void operator() ( osg::Uniform* uniform, osg::NodeVisitor* nv )
//    {
//        //float angle = 2.0 * nv->getFrameStamp()->getSimulationTime();
//        //float sine = sinf( angle );        // -1 -> 1
//        //float v01 = 0.5f * sine + 0.5f;        //  0 -> 1
//        //float v10 = 1.0f - v01;                //  1 -> 0
//
//        double  fractpart, intpart;
//        fractpart = modf (nv->getFrameStamp()->getSimulationTime() / 100.0f , &intpart);
//
//        uniform->set( osg::Vec4(/*1.505f*/1.5f, /*0.8f*v01*/1.5f, 1.5f, 100*fractpart) ); 
//    }
//};

class texturesHolder  : public texturesHolder_base
{
    osg::Texture2D *createLMTexture(int width, int height)
    {
        osg::Texture2D* texture = new osg::Texture2D;
        texture->setTextureSize(width,height);
        texture->setInternalFormat(/*GL_RGBA*/GL_RGBA16F);
        texture->setSourceFormat(GL_RGBA);
        texture->setSourceType(GL_UNSIGNED_BYTE);
        texture->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_BORDER);
        texture->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_BORDER);
        texture->setFilter(osg::Texture::MIN_FILTER,osg::Texture::LINEAR_MIPMAP_LINEAR);
        texture->setFilter(osg::Texture::MAG_FILTER,osg::Texture::LINEAR);
        texture->setBorderColor(osg::Vec4d(0,0,0,0));
        // texture->setUseHardwareMipMapGeneration(false);
        texture->setNumMipmapLevels(4);

        return texture;
    }

public:
    struct textures_t
    {
        osg::ref_ptr<osg::Texture2D>      colorTex;
        osg::ref_ptr<osg::Texture2D>      normalTex;
        osg::ref_ptr<osg::Texture2D>      nightTex;
        osg::ref_ptr<osg::Texture2D>      detailsTex;
        osg::ref_ptr<osg::TextureCubeMap> envTex;
        osg::ref_ptr<osg::Texture2D>      decalTex;
        osg::ref_ptr<osg::Texture2D>      lmTex;
    };

public:
    static inline const textures_t& Create(const mat::materials_t&  mats, std::string mat_name,std::string model_name)
    {
        if (   mat_name.find("building") !=std::string::npos
            || mat_name.find("ground")   !=std::string::npos
            || mat_name.find("tree")     !=std::string::npos
            || mat_name.find("concrete") !=std::string::npos
            || mat_name.find("sea")      !=std::string::npos
            || mat_name.find("mountain") !=std::string::npos
            || mat_name.find("railing")  !=std::string::npos
            || mat_name.find("panorama") !=std::string::npos
            || mat_name.find("plane") !=std::string::npos
            || mat_name.find("rotor") !=std::string::npos
            || mat_name.find("default") !=std::string::npos
            )
        {
            return texCreator(mats,mat_name,model_name); 
        }

        return texCreator(mats,"default",model_name);
    }

    osg::ref_ptr<osg::TextureCubeMap>   getEnvTexture()   override
    {
        return   envTex;
    }

    osg::ref_ptr<osg::Texture2D>   getDecalTexture() override
    {
        return   decalTex;
    }
    
    osg::ref_ptr<osg::Texture2D>   getLightMapTexture()  override
    {
        return   lmTex;
    }

    friend texturesHolder_base&   getTextureHolder();

private:

    osg::ref_ptr<osg::Texture2D>      detailsTex;
    osg::ref_ptr<osg::Texture2D>      emptyTex;
    osg::ref_ptr<osg::TextureCubeMap> envTex;
    osg::ref_ptr<osg::Texture2D>      decalTex;
    osg::ref_ptr<osg::Texture2D>      lmTex;

    texturesHolder()
    {          
        detailsTex = new osg::Texture2D;
        detailsTex->setImage( osgDB::readImageFile("Detail.dds",new osgDB::Options("")) );  
        detailsTex->setWrap(  osg::Texture::WRAP_S, osg::Texture::REPEAT );
        detailsTex->setWrap(  osg::Texture::WRAP_T, osg::Texture::REPEAT );
        detailsTex->setFilter(osg::Texture::MIN_FILTER,osg::Texture::LINEAR_MIPMAP_LINEAR);
        detailsTex->setMaxAnisotropy(16.0f);

        emptyTex = new osg::Texture2D;
        emptyTex->setImage( osgDB::readImageFile("empty_n.dds",new osgDB::Options("")) );  
        emptyTex->setWrap(  osg::Texture::WRAP_S, osg::Texture::REPEAT );
        emptyTex->setWrap(  osg::Texture::WRAP_T, osg::Texture::REPEAT );
        emptyTex->setFilter(osg::Texture::MIN_FILTER,osg::Texture::LINEAR_MIPMAP_LINEAR);
        emptyTex->setMaxAnisotropy(16.0f);

        decalTex = new osg::Texture2D;
        decalTex->setTextureSize(1024, 1024);
        decalTex->setInternalFormat( GL_RGBA );
        decalTex->setBorderWidth( 0 );
        decalTex->setFilter( osg::Texture::MIN_FILTER, osg::Texture::NEAREST );
        decalTex->setFilter( osg::Texture::MAG_FILTER, osg::Texture::NEAREST );

        // create and setup the texture object
        envTex = new osg::TextureCubeMap;
        const unsigned envSize = 256;
        envTex->setInternalFormat(GL_RGB);
        envTex->setTextureSize(envSize, envSize);
        envTex->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_BORDER/*REPEAT*//*CLAMP_TO_EDGE*/); // CLAMP_TO_BORDER/*CLAMP_TO_EDGE*/
        envTex->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_BORDER/*REPEAT*//*CLAMP_TO_EDGE*/);
        envTex->setWrap(osg::Texture::WRAP_R, osg::Texture::CLAMP_TO_BORDER/*REPEAT*//*CLAMP_TO_EDGE*/);
        envTex->setFilter(osg::TextureCubeMap::MIN_FILTER,osg::TextureCubeMap::LINEAR_MIPMAP_LINEAR/*LINEAR*/);
        envTex->setFilter(osg::TextureCubeMap::MAG_FILTER,osg::TextureCubeMap::LINEAR);
        envTex->setMaxAnisotropy(16.0f);
#ifndef TEST_EVN_CUBE_MAP 
        envTex->setNumMipmapLevels(3);
        envTex->setUseHardwareMipMapGeneration(true);
#endif

FIXME(Все теже кривые плоскости)
#ifdef TEST_EVN_CUBE_MAP 
#if 1
/*        envTex->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP);
        envTex->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP);
        envTex->setWrap(osg::Texture::WRAP_R, osg::Texture::CLAMP);
        envTex->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR_MIPMAP_LINEAR);
        envTex->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR); */   
        // assign the six images to the texture object
        envTex->setImage(osg::TextureCubeMap::POSITIVE_X, osgDB::readImageFile("test_posx.jpg"));
        envTex->setImage(osg::TextureCubeMap::NEGATIVE_X, osgDB::readImageFile("test_negx.jpg"));
        envTex->setImage(osg::TextureCubeMap::POSITIVE_Y, osgDB::readImageFile("test_posy.jpg"));
        envTex->setImage(osg::TextureCubeMap::NEGATIVE_Y, osgDB::readImageFile("test_negy.jpg"));
        envTex->setImage(osg::TextureCubeMap::POSITIVE_Z, osgDB::readImageFile("test_posz.jpg"));
        envTex->setImage(osg::TextureCubeMap::NEGATIVE_Z, osgDB::readImageFile("test_negz.jpg"));
        //envTex->setUseHardwareMipMapGeneration(true);
#else
        // generate the six highlight map images (light direction = [1, 1, -1])
        osgUtil::HighlightMapGenerator *mapgen = new osgUtil::HighlightMapGenerator(
            osg::Vec3(1, 1, -1),            // light direction
            osg::Vec4(1, 0.9f, 0.8f, 1),    // light color
            8);                             // specular exponent

        mapgen->generateMap();

        // assign the six images to the texture object
        envTex->setImage(osg::TextureCubeMap::POSITIVE_X, mapgen->getImage(osg::TextureCubeMap::POSITIVE_X));
        envTex->setImage(osg::TextureCubeMap::NEGATIVE_X, mapgen->getImage(osg::TextureCubeMap::NEGATIVE_X));
        envTex->setImage(osg::TextureCubeMap::POSITIVE_Y, mapgen->getImage(osg::TextureCubeMap::POSITIVE_Y));
        envTex->setImage(osg::TextureCubeMap::NEGATIVE_Y, mapgen->getImage(osg::TextureCubeMap::NEGATIVE_Y));
        envTex->setImage(osg::TextureCubeMap::POSITIVE_Z, mapgen->getImage(osg::TextureCubeMap::POSITIVE_Z));
        envTex->setImage(osg::TextureCubeMap::NEGATIVE_Z, mapgen->getImage(osg::TextureCubeMap::NEGATIVE_Z));
#endif
#endif

        lmTex = createLMTexture(1024, 1024);
    }


    static texturesHolder& getTextureHolder()
    {
        static texturesHolder th;
        return th;
    }

    static inline const textures_t&  texCreator(const mat::materials_t&  mats, std::string mat_name,std::string model_name)
    {
        texturesHolder& th = getTextureHolder();

        if(GetTextures().find(mat_name)==GetTextures().end())
        {
            FIXME(test it);

            textures_t  t /*= th*/; 
            t.colorTex = new osg::Texture2D;
            t.nightTex = new osg::Texture2D;
            t.normalTex  = new osg::Texture2D;
            t.detailsTex = th.detailsTex;
            t.envTex = th.envTex;
            t.lmTex  = th.lmTex;

            auto range = mats.equal_range(mat_name);

            bool night_tex  = false; 
            bool normal_tex = false;

            for (auto it = range.first; it != range.second; ++it)
            {
                osgDB::FilePathList fpl = osgDB::getDataFilePathList();
                
                std::for_each(fpl.begin(),fpl.end(),[=](std::string& path){path += "/" + model_name; });

                std::string name = osgDB::findFileInPath(it->second.path, fpl,osgDB::CASE_INSENSITIVE);

                if(it->second.unit == 0) 
                {   
                    auto imf = osgDB::readImageFile(name);
                    t.colorTex->setImage( imf );
                    t.colorTex->setWrap(  osg::Texture::WRAP_S, it->second.wrap_s/*osg::Texture::REPEAT*/ );
                    t.colorTex->setWrap(  osg::Texture::WRAP_T, it->second.wrap_t/*osg::Texture::REPEAT*/ );
                    t.colorTex->setFilter(osg::Texture::MIN_FILTER,osg::Texture::LINEAR_MIPMAP_LINEAR);
                    t.colorTex->setMaxAnisotropy(16.0f);

                    // Существенной разницы не заметно метров 40-60 виртуальной и 10-20 графической
                    //t.colorTex->setInternalFormatMode(
                    //    osg::Texture2D::USE_S3TC_DXT1_COMPRESSION );
                    //t.colorTex->setUnRefImageDataAfterApply( true );
                } 
                else
                if(it->second.unit == 1) 
                {   
                    auto imf = osgDB::readImageFile(name);
                    t.normalTex->setImage( imf );
                    t.normalTex->setWrap(  osg::Texture::WRAP_S, it->second.wrap_s/*osg::Texture::CLAMP*/ );
                    t.normalTex->setWrap(  osg::Texture::WRAP_T, it->second.wrap_t/*osg::Texture::CLAMP*/ );
                    t.normalTex->setFilter(osg::Texture::MIN_FILTER,osg::Texture::LINEAR_MIPMAP_LINEAR);
                    t.normalTex->setMaxAnisotropy(16.0f);

                    normal_tex = true;
                } 
                else 	
                if(it->second.unit == 2)
                {
                    t.nightTex->setImage( osgDB::readImageFile(name) );
                    t.nightTex->setWrap(  osg::Texture::WRAP_S, it->second.wrap_s/*osg::Texture::REPEAT*/ );
                    t.nightTex->setWrap(  osg::Texture::WRAP_T, it->second.wrap_t/*osg::Texture::REPEAT*/ );
                    t.nightTex->setFilter(osg::Texture::MIN_FILTER,osg::Texture::LINEAR_MIPMAP_LINEAR);
                    t.nightTex->setMaxAnisotropy(16.0f);

                    // Существенной разницы не заметно метров 40-60 виртуальной и 10-20 графической
                    //t.nightTex->setInternalFormatMode(
                    //    osg::Texture2D::USE_S3TC_DXT1_COMPRESSION );
                    //t.nightTex->setUnRefImageDataAfterApply( true );

                    night_tex = true;
                }
            }

            if(!night_tex)
                t.nightTex = th.emptyTex;  

            if(!normal_tex)
                t.normalTex = th.emptyTex;



            GetTextures()[mat_name] = t;
        }

        return  GetTextures()[mat_name];
    }

    static inline std::map<std::string,textures_t>& GetTextures()
    {
        static std::map<std::string,textures_t>     textures;
        return textures;
    }

};

texturesHolder_base&   getTextureHolder()
{
    return texturesHolder::getTextureHolder();
}

std::string osg_modification(uint16_t version, const std::string& prog)
{
      std::string source ( prog );
      if(version>130)
      {   
          const char* header_mat = {
          "uniform mat4 osg_ModelViewMatrix; \n"
          "uniform mat4 osg_ModelViewProjectionMatrix; \n"
          "uniform mat4 osg_ProjectionMatrix; \n"
          "uniform mat3 osg_NormalMatrix; \n"
          };
          
          const char* header_attr[] = {
              "in vec4 osg_Vertex;\n",
              "in vec3 osg_Normal;\n",
              "in vec4 osg_Color;\n",
              "in vec4 osg_SecondaryColor; \n",
              "in vec4 osg_FogCoord; \n",
              "in vec2 osg_MultiTexCoord0; \n",
              "in vec2 osg_MultiTexCoord1; \n",
              "in vec2 osg_MultiTexCoord2; \n",
              "in vec2 osg_MultiTexCoord3; \n",
              "in vec2 osg_MultiTexCoord4; \n",
              "in vec2 osg_MultiTexCoord5; \n",
              "in vec2 osg_MultiTexCoord6; \n",
              "in vec2 osg_MultiTexCoord7; \n"
          };

          uint16_t cattr = 13; 
          cattr = utils::replaceAll(source,std::string("gl_Vertex"), std::string("osg_Vertex"))>0?1:0;
          cattr = utils::replaceAll(source,std::string("gl_Normal"), std::string("osg_Normal"))>0?2:cattr;
          cattr = utils::replaceAll(source,std::string("gl_Color"), std::string("osg_Color"))>0?3:cattr;
          cattr = utils::replaceAll(source,std::string("gl_SecondaryColor"), std::string("osg_SecondaryColor"))>0?4:cattr;
          cattr = utils::replaceAll(source,std::string("gl_FogCoord"), std::string("osg_FogCoord"))>0?5:cattr;
          cattr = utils::replaceAll(source,std::string("gl_MultiTexCoord"), std::string("osg_MultiTexCoord"))>0?8:cattr;

          utils::replaceAll(source,std::string("gl_ModelViewMatrix"), std::string("osg_ModelViewMatrix"));
          utils::replaceAll(source,std::string("gl_ModelViewProjectionMatrix"), std::string("osg_ModelViewProjectionMatrix"));
          utils::replaceAll(source,std::string("gl_ProjectionMatrix"), std::string("osg_ProjectionMatrix"));
          
          std::string header;
          for (uint16_t i =0;i< cattr; ++i )
          {
              header += header_attr[i];
          }


          source = std::string(header_mat) + header
                                           + source;

      }

      return source;
}

class programsHolder: public programsHolder_base
{

public:
    static inline const program_t& Create(std::string mat_name)
    {
        const uint16_t version = GLSL_VERSION;
        const std::string  comp_str = GLSL_VERSION>400? " compatibility":"";

        if(GetPrograms().find(mat_name)==GetPrograms().end())
        {
            program_t p;
            p.program = new osg::Program;
            p.program->setName(mat_name);
            
            if(GetShader(shaders::VS,mat_name))
            {
                std::string prog = "#version " + boost::lexical_cast<string>(version) +  comp_str + "\n " 
                    + osg_modification(version,utils::format(*GetShader(shaders::VS,mat_name)));
                auto vs = new osg::Shader( osg::Shader::VERTEX, prog );
                p.program->addShader( vs );

            }

            if(GetShader(shaders::FS,mat_name))
            {
                std::string prog = "#version " + boost::lexical_cast<string>(version)  +  comp_str +"\n "
                    + osg_modification(version,utils::format(*GetShader(shaders::FS,mat_name)));
                auto fs = new osg::Shader(osg::Shader::FRAGMENT, prog);
                p.program->addShader( fs );
                
            }

            p.program->addBindAttribLocation( "tangent" , 6 );
            p.program->addBindAttribLocation( "binormal", 7 );

            GetPrograms()[mat_name]=p;
        }

        return GetPrograms()[mat_name];
    }

    static inline std::string GetMaterialName( const std::string& mat_name )
    {
         std::string mat_name_cut = mat_name.substr(0, mat_name.find("_"));
         FIXME( "Есть где-то buildingtrack" ) 

         if (mat_name.find("building") !=std::string::npos)
         {
              return"building"; 
         }

         return mat_name_cut;
    }

private:
    
    static inline boost::optional<std::string> GetShader(const shaders::shader_t& t, const std::string& mat_name)
    {
            const char* shader = GetShader_internal( t,  mat_name);

            if(shader)
                return std::string(shader);

            return boost::none;
    }

    static inline const char* GetShader_internal(const shaders::shader_t& t, const std::string& mat_name)
    {
        std::string mat_name_cut = GetMaterialName(mat_name);

        auto fp = fn_reg::function<const char*(shaders::shader_t)>(mat_name_cut);
        
        if (fp)
            return fp(t);

        //FIXME( "Есть где-то buildingtrack" ) 

        //if (mat_name.find("building") !=std::string::npos)
        //{
        //    return shaders::building_mat::get_shader(t); 
        //}

        if (mat_name.find("default") !=std::string::npos)
        {
            return shaders::default_mat::get_shader(t);  
        }

        return nullptr;

    }



    static inline std::map<std::string,program_t>& GetPrograms()
    {
        static std::map<std::string,program_t>     programs;
        return programs;
    }

};


void computeAttributes(osg::Node* model,std::string mat_name)
{
    if (   mat_name.find("ground")   !=std::string::npos   
        || mat_name.find("sea")      !=std::string::npos     
        || mat_name.find("mountain") !=std::string::npos 
        || mat_name.find("concrete") !=std::string::npos 
        || mat_name.find("default")  !=std::string::npos
        || mat_name.find("plane")  !=std::string::npos 
        )
    {
        ComputeTangentVisitor ctv;
        ctv.setTraversalMode( osg::NodeVisitor::TRAVERSE_ALL_CHILDREN );
        model->accept( ctv );
    }
}


programsHolder_base::program_t  createProgram(std::string mat_name)
{
    return programsHolder::Create(mat_name);
}


void createMaterial(osg::StateSet* stateset,std::string model_name,std::string mat_name,const mat::materials_t& m)
{
    texturesHolder::textures_t t = texturesHolder::Create(m,mat_name,model_name);
    programsHolder::program_t  p = programsHolder::Create(mat_name);

    const osg::StateSet::UniformList& ul = stateset->getUniformList();
    for(osg::StateSet::UniformList::const_iterator itr = ul.begin();
        itr != ul.end();
        ++itr)
    {   
        std::string name = itr->first;
        // pcp->apply(*(itr->second.first));
    }    

    stateset->addUniform( new osg::Uniform("colorTex"      , BASE_COLOR_TEXTURE_UNIT) );
    stateset->addUniform( new osg::Uniform("normalTex"     , BASE_NORMAL_TEXTURE_UNIT) ); 
    stateset->addUniform( new osg::Uniform("nightTex"      , BASE_NIGHT_TEXTURE_UNIT) );
    stateset->addUniform( new osg::Uniform("detailTex"     , BASE_DETAILS_TEXTURE_UNIT) ); 
    stateset->addUniform( new osg::Uniform("envTex"        , BASE_ENV_TEXTURE_UNIT) ); 
    stateset->addUniform( new osg::Uniform("ViewDecalMap"  , BASE_DECAL_TEXTURE_UNIT) );
    stateset->addUniform( new osg::Uniform("ViewLightMap"  , BASE_LM_TEXTURE_UNIT) );
    // Добавляется в VDS Map
    //stateset->addUniform( new osg::Uniform("shadowTexture0", BASE_SHADOW_TEXTURE_UNIT) );


    stateset->setAttributeAndModes( p.program.get() );

    //if ( mat_name.find("panorama") !=std::string::npos )
    //{     
    //    osg::Uniform* uni_fog =  new osg::Uniform("fog_params", osg::Vec4(0.0f, 0.0f, 0.0f, 0.0f));
    //    stateset->addUniform( uni_fog );
    //    uni_fog->setUpdateCallback(new FogCallback);
    //    uni_fog->setDataVariance(osg::Object::DYNAMIC);
    //}
 
    std::ifstream alpha_file(osgDB::findDataFile(programsHolder::GetMaterialName(mat_name) + "\\alpha_to_coverage.on"));

    if ( alpha_file.good() )
    { 
        stateset->setMode(GL_SAMPLE_ALPHA_TO_COVERAGE,osg::StateAttribute::ON);               
    }

    osg::StateAttribute::GLModeValue value = osg::StateAttribute::ON|osg::StateAttribute::OVERRIDE;
    stateset->setTextureAttributeAndModes( BASE_COLOR_TEXTURE_UNIT, t.colorTex.get(), value );
    stateset->setTextureAttributeAndModes( BASE_NORMAL_TEXTURE_UNIT, t.normalTex.get(), value );
    stateset->setTextureAttributeAndModes( BASE_NIGHT_TEXTURE_UNIT, t.nightTex.get(), value );

    stateset->setTextureAttributeAndModes( BASE_DETAILS_TEXTURE_UNIT, t.detailsTex.get(), value );
    stateset->setTextureAttributeAndModes( BASE_ENV_TEXTURE_UNIT, t.envTex.get(), value );
    stateset->setTextureAttributeAndModes( BASE_DECAL_TEXTURE_UNIT, getTextureHolder().getDecalTexture().get(), value ); 
    stateset->setTextureAttributeAndModes( BASE_LM_TEXTURE_UNIT, getTextureHolder().getLightMapTexture().get(), value ); 
    
    stateset->setMode(GL_TEXTURE_CUBE_MAP_SEAMLESS_ARB, osg::StateAttribute::ON); 
}


void createMaterialLite(osg::StateSet* stateset,std::string model_name,std::string mat_name,const mat::materials_t& m)
{
    texturesHolder::textures_t t = texturesHolder::Create(m,mat_name,model_name);
   
    osg::StateAttribute::GLModeValue value = osg::StateAttribute::ON|osg::StateAttribute::OVERRIDE;
    stateset->setTextureAttributeAndModes( BASE_COLOR_TEXTURE_UNIT , t.colorTex.get(), value );
    stateset->setTextureAttributeAndModes( BASE_NORMAL_TEXTURE_UNIT, t.normalTex.get(), value );
    stateset->setTextureAttributeAndModes( BASE_NIGHT_TEXTURE_UNIT , t.nightTex.get(), value );

    stateset->setTextureAttributeAndModes( BASE_DETAILS_TEXTURE_UNIT, t.detailsTex.get(), value );
    stateset->setTextureAttributeAndModes( BASE_ENV_TEXTURE_UNIT    , t.envTex.get(), value );
    stateset->setTextureAttributeAndModes( BASE_DECAL_TEXTURE_UNIT  , getTextureHolder().getDecalTexture().get(), value ); 
    stateset->setTextureAttributeAndModes( BASE_LM_TEXTURE_UNIT     , getTextureHolder().getLightMapTexture().get(), value ); 

    stateset->setMode(GL_TEXTURE_CUBE_MAP_SEAMLESS_ARB, osg::StateAttribute::ON); 
}

} //namespace creators 


namespace mat
{
    inline osg::Texture::WrapMode getWrapMode(std::string wrap)
    {
        if (wrap == "repeat")
            return osg::Texture::REPEAT;
        if (wrap == "mirror")
            return osg::Texture::MIRROR;
        if (wrap == "clamp")
            return osg::Texture::CLAMP;
        if (wrap == "clamp_to_edge")
            return osg::Texture::CLAMP_TO_EDGE;
        if (wrap == "clamp_to_border")
            return osg::Texture::CLAMP_TO_BORDER;
        
        OSG_WARN << "Unrecognized WrapMode." << std::endl;
        return osg::Texture::CLAMP;
    }

    reader::reader()
    {
    }

    reader::reader(std::string full_path)
    {
        mats_ = read (full_path);
    }

    materials_t  reader::read (std::string full_path)
    {
        pugi::xml_document doc;
        materials_t mats_;

        std::string in_file_name = /*fs::path(*/full_path/*).leaf()*/ /*+ ".mat.xml"*/;

        bool l = doc.load_file(in_file_name.c_str());
        if(l)
        {
            pugi::xml_node root = doc.child("root");

            for (pugi::xml_node m = root.first_child(); m; m = m.next_sibling())
            {
                for (pugi::xml_node t = m.first_child(); t; t = t.next_sibling())
                {
                    texture_t tex;
                    tex.path = t.attribute("path").as_string();
                    tex.unit = t.attribute("unit").as_int();
                    tex.wrap_s =  getWrapMode(t.attribute("wrap_s").as_string());
                    tex.wrap_t =  getWrapMode(t.attribute("wrap_t").as_string());
                    mats_.insert(materials_t::value_type(std::string(m.attribute("name").as_string()),tex));
                }
            }	

        }
        else
            std::cerr << "File not found: " << in_file_name;

        return mats_;
    }

}
