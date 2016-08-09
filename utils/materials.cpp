#include "stdafx.h"
#include "av/precompiled.h"
#include "materials.h"
#include "av/shaders.h"
#include "av/avCore/Utils.h"
#include "av/avCore/Noise.h"

#include "visitors/ct_visitor.h"




namespace Database
{
    bool LoadShaderInternal(const std::string & fileName, std::stringstream& file, std::ostream & text );
}

//
//  ext
//
#include "pugixml.hpp"

namespace creators 
{


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

	osg::Texture2D * createTexture(int width, int height)
	{
		osg::Texture2D* texture = new osg::Texture2D;
		texture->setTextureSize(width,height);
		texture->setInternalFormat(GL_RGBA);
		texture->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
		texture->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
		texture->setFilter(osg::Texture::MIN_FILTER,osg::Texture::LINEAR/*LINEAR_MIPMAP_LINEAR*/);
		texture->setFilter(osg::Texture::MAG_FILTER,osg::Texture::LINEAR);
		texture->setUseHardwareMipMapGeneration(false);
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
		osg::ref_ptr<osg::Texture2D>      reflTex;
    };

public:
    static inline const textures_t& Create(const mat::materials_t&  mats, const std::string& mat_name,const std::string& model_name)
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
        return   texs_.envTex;
    }

    osg::ref_ptr<osg::Texture2D>   getDecalTexture() override
    {
        return   texs_.decalTex;
    }
    
    osg::ref_ptr<osg::Texture2D>   getLightMapTexture()  override
    {
        return   texs_.lmTex;
	}

	osg::ref_ptr<osg::Texture2D>   getReflTexture()  override
	{
		return   texs_.reflTex;
	}

    friend texturesHolder_base&   getTextureHolder();

private:

	textures_t					      texs_;
	osg::ref_ptr<osg::Texture2D>      emptyTex;

    texturesHolder()
    {    

		emptyTex = new osg::Texture2D;
		emptyTex->setImage( osgDB::readImageFile("empty_n.dds",new osgDB::Options("")) );  
		emptyTex->setWrap(  osg::Texture::WRAP_S, osg::Texture::REPEAT );
		emptyTex->setWrap(  osg::Texture::WRAP_T, osg::Texture::REPEAT );
		emptyTex->setFilter(osg::Texture::MIN_FILTER,osg::Texture::LINEAR_MIPMAP_LINEAR);
		emptyTex->setMaxAnisotropy(16.0f);

        texs_.detailsTex = new osg::Texture2D;
        texs_.detailsTex->setImage( osgDB::readImageFile("Detail.dds",new osgDB::Options("")) );  
        texs_.detailsTex->setWrap(  osg::Texture::WRAP_S, osg::Texture::REPEAT );
        texs_.detailsTex->setWrap(  osg::Texture::WRAP_T, osg::Texture::REPEAT );
        texs_.detailsTex->setFilter(osg::Texture::MIN_FILTER,osg::Texture::LINEAR_MIPMAP_LINEAR);
        texs_.detailsTex->setMaxAnisotropy(16.0f);

        texs_.decalTex = new osg::Texture2D;
        texs_.decalTex->setTextureSize(1024,1024);
        texs_.decalTex->setInternalFormat(/*GL_RGBA*/GL_RGBA8);
        texs_.decalTex->setSourceFormat(GL_RGBA);
        texs_.decalTex->setSourceType(GL_UNSIGNED_BYTE);
        texs_.decalTex->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_BORDER);
        texs_.decalTex->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_BORDER);
        texs_.decalTex->setFilter(osg::Texture::MIN_FILTER,osg::Texture::LINEAR_MIPMAP_LINEAR);
        texs_.decalTex->setFilter(osg::Texture::MAG_FILTER,osg::Texture::LINEAR);
        texs_.decalTex->setBorderColor(osg::Vec4d(0,0,0,0));
        //texs_.decalTex->setBorderWidth( 0 );
        texs_.decalTex->setUseHardwareMipMapGeneration(false);
        // texs_.decalTex->setNumMipmapLevels(5);
        texs_.detailsTex->setMaxAnisotropy(16.0f);

        // create and setup the texture object
        texs_.envTex = new osg::TextureCubeMap;
        const unsigned envSize = 256;
        texs_.envTex->setInternalFormat(GL_RGB);
        texs_.envTex->setTextureSize(envSize, envSize);
        texs_.envTex->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_BORDER/*REPEAT*//*CLAMP_TO_EDGE*/); // CLAMP_TO_BORDER/*CLAMP_TO_EDGE*/
        texs_.envTex->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_BORDER/*REPEAT*//*CLAMP_TO_EDGE*/);
        texs_.envTex->setWrap(osg::Texture::WRAP_R, osg::Texture::CLAMP_TO_BORDER/*REPEAT*//*CLAMP_TO_EDGE*/);
        texs_.envTex->setFilter(osg::TextureCubeMap::MIN_FILTER,osg::TextureCubeMap::LINEAR_MIPMAP_LINEAR/*LINEAR*/);
        texs_.envTex->setFilter(osg::TextureCubeMap::MAG_FILTER,osg::TextureCubeMap::LINEAR);
        texs_.envTex->setMaxAnisotropy(16.0f);
#ifndef TEST_EVN_CUBE_MAP 
        texs_.envTex->setNumMipmapLevels(3);
        texs_.envTex->setUseHardwareMipMapGeneration(true);
#endif

FIXME(Все те же кривые плоскости)
#ifdef TEST_EVN_CUBE_MAP 
#if 1
/*        texs_.envTex->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP);
        texs_.envTex->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP);
        texs_.envTex->setWrap(osg::Texture::WRAP_R, osg::Texture::CLAMP);
        texs_.envTex->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR_MIPMAP_LINEAR);
        texs_.envTex->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR); */   
        // assign the six images to the texture object
        texs_.envTex->setImage(osg::TextureCubeMap::POSITIVE_X, osgDB::readImageFile("test_posx.jpg"));
        texs_.envTex->setImage(osg::TextureCubeMap::NEGATIVE_X, osgDB::readImageFile("test_negx.jpg"));
        texs_.envTex->setImage(osg::TextureCubeMap::POSITIVE_Y, osgDB::readImageFile("test_posy.jpg"));
        texs_.envTex->setImage(osg::TextureCubeMap::NEGATIVE_Y, osgDB::readImageFile("test_negy.jpg"));
        texs_.envTex->setImage(osg::TextureCubeMap::POSITIVE_Z, osgDB::readImageFile("test_posz.jpg"));
        texs_.envTex->setImage(osg::TextureCubeMap::NEGATIVE_Z, osgDB::readImageFile("test_negz.jpg"));
        //texs_.envTex->setUseHardwareMipMapGeneration(true);
#else
        // generate the six highlight map images (light direction = [1, 1, -1])
        osgUtil::HighlightMapGenerator *mapgen = new osgUtil::HighlightMapGenerator(
            osg::Vec3(1, 1, -1),            // light direction
            osg::Vec4(1, 0.9f, 0.8f, 1),    // light color
            8);                             // specular exponent

        mapgen->generateMap();

        // assign the six images to the texture object
        texs_.envTex->setImage(osg::TextureCubeMap::POSITIVE_X, mapgen->getImage(osg::TextureCubeMap::POSITIVE_X));
        texs_.envTex->setImage(osg::TextureCubeMap::NEGATIVE_X, mapgen->getImage(osg::TextureCubeMap::NEGATIVE_X));
        texs_.envTex->setImage(osg::TextureCubeMap::POSITIVE_Y, mapgen->getImage(osg::TextureCubeMap::POSITIVE_Y));
        texs_.envTex->setImage(osg::TextureCubeMap::NEGATIVE_Y, mapgen->getImage(osg::TextureCubeMap::NEGATIVE_Y));
        texs_.envTex->setImage(osg::TextureCubeMap::POSITIVE_Z, mapgen->getImage(osg::TextureCubeMap::POSITIVE_Z));
        texs_.envTex->setImage(osg::TextureCubeMap::NEGATIVE_Z, mapgen->getImage(osg::TextureCubeMap::NEGATIVE_Z));
#endif
#endif

        texs_.lmTex = createLMTexture( 1024, 1024 );
		 
		texs_.reflTex = createTexture( 512, 512 );
    }


    static texturesHolder& getTextureHolder()
    {
        static texturesHolder th;
        return th;
    }

    static inline const textures_t&  texCreator(const mat::materials_t&  mats, const std::string& mat_name,const std::string& model_name)
    {
        texturesHolder& th = getTextureHolder();

        if(GetTextures().find(mat_name)==GetTextures().end())
        {
            textures_t  t /*= th*/; 
            t.colorTex = new osg::Texture2D;
            t.nightTex = new osg::Texture2D;
            t.normalTex  = new osg::Texture2D;
            t.detailsTex = th.texs_.detailsTex;
            t.envTex = th.texs_.envTex;
            t.lmTex  = th.texs_.lmTex;

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
                    t.colorTex->setUnRefImageDataAfterApply( true );
                } 
                else
                if(it->second.unit == 1) 
                {   
                    if(it->second.path=="generate_noise")
                       t.normalTex = avCore::Noise::generate2DTex(/*baseFreq*//*4.0f*/32.0f, /*persistence*/0.5, /*w*/512, /*h*/512, /*periodic*/true);
                    else
                    {
                        auto imf = osgDB::readImageFile(name);
                        t.normalTex->setImage( imf );
                        t.normalTex->setWrap(  osg::Texture::WRAP_S, it->second.wrap_s/*osg::Texture::CLAMP*/ );
                        t.normalTex->setWrap(  osg::Texture::WRAP_T, it->second.wrap_t/*osg::Texture::CLAMP*/ );
                        t.normalTex->setFilter(osg::Texture::MIN_FILTER,osg::Texture::LINEAR_MIPMAP_LINEAR);
                        t.normalTex->setMaxAnisotropy(16.0f);
                    }


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
          cattr = Utils::replaceAll(source,std::string("gl_Vertex"), std::string("osg_Vertex"))>0?1:0;
          cattr = Utils::replaceAll(source,std::string("gl_Normal"), std::string("osg_Normal"))>0?2:cattr;
          cattr = Utils::replaceAll(source,std::string("gl_Color"), std::string("osg_Color"))>0?3:cattr;
          cattr = Utils::replaceAll(source,std::string("gl_SecondaryColor"), std::string("osg_SecondaryColor"))>0?4:cattr;
          cattr = Utils::replaceAll(source,std::string("gl_FogCoord"), std::string("osg_FogCoord"))>0?5:cattr;
          cattr = Utils::replaceAll(source,std::string("gl_MultiTexCoord"), std::string("osg_MultiTexCoord"))>0?8:cattr;

          Utils::replaceAll(source,std::string("gl_ModelViewMatrix"), std::string("osg_ModelViewMatrix"));
          Utils::replaceAll(source,std::string("gl_ModelViewProjectionMatrix"), std::string("osg_ModelViewProjectionMatrix"));
          Utils::replaceAll(source,std::string("gl_ProjectionMatrix"), std::string("osg_ProjectionMatrix"));
          
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
    static inline const program_t& Create(const std::string& mat_name , const std::string& mat_suffix, const std::string& preprocessorDefinitions = std::string())
    {
        const uint16_t version = GLSL_VERSION;
        const std::string  comp_str = GLSL_VERSION>=150 ?" compatibility":"";

        const std::string  mat_name_cut = GetMaterialName(mat_name) + (mat_suffix.size()>0?(mat_suffix):"");

        if(GetPrograms().find(mat_name_cut)==GetPrograms().end())
        {
            program_t p;
            p.program = new osg::Program;
            p.program->setName(mat_name_cut);

            static osg::Shader* ssv = AddShader(shaders::VS, "shadow", version, comp_str, string());
            static osg::Shader* ssf = AddShader(shaders::FS, "shadow", version, comp_str, string());

            p.program->addShader( ssv );
            p.program->addShader( ssf );
            
            p.program->addShader( AddShader(shaders::VS, mat_name_cut, version, comp_str, preprocessorDefinitions));
            p.program->addShader( AddShader(shaders::FS, mat_name_cut, version, comp_str, preprocessorDefinitions));

            p.program->addBindAttribLocation( "tangent" , 6 );
            p.program->addBindAttribLocation( "binormal", 7 );

            GetPrograms()[mat_name_cut]=p;
        }

        return GetPrograms()[mat_name_cut];
    }

    static osg::Shader* AddShader( const shaders::shader_t& t, std::string mat_name, const uint16_t version, const std::string comp_str, const std::string& preprocessorDefinitions ) 
    {
        auto shader_text = GetShader(t,mat_name);

        if(shader_text)
        {
            std::string prog = "#version " + boost::lexical_cast<string>(version) +  comp_str + "\n " 
                + preprocessorDefinitions                                                     + "\n " 
                + osg_modification(version,Utils::format(*shader_text));
            return new osg::Shader( static_cast<osg::Shader::Type>(t), prog );

        }
        return new osg::Shader();
    }


    static inline std::string GetMaterialName( const std::string& mat_name )
    {
         std::string mat_name_cut = mat_name.substr(0, mat_name.find("_"));
         FIXME( "Есть где-то buildingtrack" ) 

         if (mat_name.find("building") !=std::string::npos && mat_name.find("buildinginst") ==std::string::npos)
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
            {
                std::stringstream buffer(shader);
                std::ostringstream shaderText;
                Database::LoadShaderInternal(mat_name, buffer, shaderText);
                return /*std::string(shader)*/shaderText.str();
            }
            return boost::none;
    }

    static inline const char* GetShader_internal(const shaders::shader_t& t, const std::string& mat_name)
    {
        std::string mat_name_cut = GetMaterialName(mat_name);
        
        if (auto fp = fn_reg::function<const char*(shaders::shader_t)>(mat_name_cut))
            return fp(t);

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
        || mat_name.find("color")  !=std::string::npos 
        )
    {
        ComputeTangentVisitor ctv;
        ctv.setTraversalMode( osg::NodeVisitor::TRAVERSE_ALL_CHILDREN );
        model->accept( ctv );
    }
}


programsHolder_base::program_t  createProgram(const std::string& mat_name, const std::string& preprocessorDefinitions)
{
    return programsHolder::Create(mat_name, preprocessorDefinitions);
}


void createMaterial(osg::Node* node, osg::StateSet* stateset,const std::string& model_name,const std::string& mat_name, const std::string& mat_suffix,const mat::materials_t& m)
{
    const std::string& mat_name_low = boost::to_lower_copy(mat_name);   

    texturesHolder::textures_t t = texturesHolder::Create(m,mat_name,model_name);
    programsHolder::program_t  p = programsHolder::Create(mat_name_low, mat_suffix);
    
#if 0
    force_log fl;
    LOG_ODS_MSG( "createMaterial for node: " << node->getName()  << " stateset " << stateset->getName() << " material name " << mat_name << 
                 " low = "<< mat_name_low <<  "\n");
#endif

#if 0
    const osg::StateSet::UniformList& ul = stateset->getUniformList();
    for(osg::StateSet::UniformList::const_iterator itr = ul.begin();
        itr != ul.end();
        ++itr)
    {   
        std::string name = itr->first;
        // pcp->apply(*(itr->second.first));
    } 
#endif   

    if (mat_name.find("tree")   !=std::string::npos)
			stateset->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);

#if 0
    if (/*mat_name.find("tree")   !=std::string::npos
        ||*/ mat_name.find("building") !=std::string::npos 
        )
        node->setNodeMask(0);
#endif

    FIXME(И еще немного хардкода как alpha2covrage оформить)
    if (       mat_name.find("ground")   !=std::string::npos   
            || mat_name.find("sea")      !=std::string::npos     
            || mat_name.find("mountain") !=std::string::npos 
            || mat_name.find("concrete") !=std::string::npos 
            )
            node->setNodeMask(~REFLECTION_MASK);
    
    stateset->addUniform( new osg::Uniform("colorTex"      , BASE_COLOR_TEXTURE_UNIT) );
    stateset->addUniform( new osg::Uniform("normalTex"     , BASE_NORMAL_TEXTURE_UNIT) ); 
    stateset->addUniform( new osg::Uniform("nightTex"      , BASE_NIGHT_TEXTURE_UNIT) );
    stateset->addUniform( new osg::Uniform("detailTex"     , BASE_DETAILS_TEXTURE_UNIT) ); 
    stateset->addUniform( new osg::Uniform("envTex"        , BASE_ENV_TEXTURE_UNIT) ); 
    stateset->addUniform( new osg::Uniform("ViewDecalMap"  , BASE_DECAL_TEXTURE_UNIT) );
    stateset->addUniform( new osg::Uniform("ViewLightMap"  , BASE_LM_TEXTURE_UNIT) );
    // Добавляется в VDSM
    //stateset->addUniform( new osg::Uniform("shadowTexture0", BASE_SHADOW_TEXTURE_UNIT) );

	osg::Material *osgmat = static_cast<osg::Material*>(stateset->getAttribute( osg::StateAttribute::MATERIAL ));
	if ( osgmat != NULL && (mat_name.find("statmat") !=std::string::npos || mat_name_low.find("color") !=std::string::npos ))
	{
		const osg::Vec4 &eCol = osgmat->getEmissionFrontAndBack()?osgmat->getEmission( osg::Material::FRONT_AND_BACK ):osgmat->getEmission( osg::Material::FRONT );
		const osg::Vec4 &aCol = osgmat->getAmbientFrontAndBack()?osgmat->getAmbient( osg::Material::FRONT_AND_BACK ):osgmat->getAmbient( osg::Material::FRONT );
		const osg::Vec4 &dCol = osgmat->getDiffuseFrontAndBack()?osgmat->getDiffuse( osg::Material::FRONT_AND_BACK ):osgmat->getDiffuse( osg::Material::FRONT );
		const osg::Vec4 &sCol = osgmat->getSpecularFrontAndBack()?osgmat->getSpecular( osg::Material::FRONT_AND_BACK ):osgmat->getSpecular( osg::Material::FRONT );
        const float shininess = osgmat->getShininessFrontAndBack()?osgmat->getShininess( osg::Material::FRONT_AND_BACK ):osgmat->getShininess( osg::Material::FRONT );

        stateset->addUniform( new osg::Uniform("eCol"     , eCol), osg::StateAttribute::ON | osg::StateAttribute::PROTECTED );
		stateset->addUniform( new osg::Uniform("aCol"     , aCol), osg::StateAttribute::ON | osg::StateAttribute::PROTECTED ); 
		stateset->addUniform( new osg::Uniform("dCol"     , dCol), osg::StateAttribute::ON | osg::StateAttribute::PROTECTED );
		stateset->addUniform( new osg::Uniform("sCol"     , sCol), osg::StateAttribute::ON | osg::StateAttribute::PROTECTED ); 
	}
	
	if ( osgmat != NULL)
	{
		stateset->removeAttribute(osgmat);
	}

	osg::Material *osglm = static_cast<osg::Material*>(stateset->getAttribute( osg::StateAttribute::LIGHTMODEL ));
	if ( osglm != NULL )
	{
     	stateset->removeAttribute(osglm);
	}
	
	osg::Material *osgte = static_cast<osg::Material*>(stateset->getAttribute( osg::StateAttribute::TEXENV ));
	if ( osgte != NULL )
	{
		stateset->removeAttribute(osgte);
	}

    stateset->setAttributeAndModes( p.program.get(), osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);

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


void createMaterialLite(osg::Node* node,osg::StateSet* stateset,const std::string& model_name,const std::string& mat_name,const mat::materials_t& m)
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
		init_();
    }

    reader::reader(std::string full_path)
    {
        init_();
		mats_ = read (full_path);
    }

	void reader::init_()
	{
		valid_mats_.insert("building");
		valid_mats_.insert("default");
		valid_mats_.insert("tree");
		valid_mats_.insert("ground"); 
		valid_mats_.insert("concrete");
		valid_mats_.insert("mountain");
		valid_mats_.insert("sea");
		valid_mats_.insert("railing");
		valid_mats_.insert("panorama");
		valid_mats_.insert("plane");
		valid_mats_.insert("rotor"); 
	}

    materials_t  reader::read (std::string full_path)
    {
        pugi::xml_document doc;
        materials_t mats_;

		bool l = doc.load_file(full_path.c_str());
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

					std::string name = std::string(m.attribute("name").as_string());
					
					bool matching = valid_mats_.end() != std::find_if(valid_mats_.begin(),valid_mats_.end(),[&](const std::string& name_)->bool {
						return name.find(name_) !=std::string::npos; }
					);

					if(!matching)
						name = "default_" + name;

				    mats_.insert(materials_t::value_type(name,tex));
                }
            }	

        }
        else
            std::cerr << "File not found: " << full_path;

        return mats_;
    }

}
