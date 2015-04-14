#include "stdafx.h"
#include "materials.h"
#include "av/shaders.h"
#include "visitors/ct_visitor.h"
#include "pugixml.hpp"

namespace creators 
{
class FogCallback: public osg::Uniform::Callback
{
public:
    virtual void operator() ( osg::Uniform* uniform, osg::NodeVisitor* nv )
    {
        //float angle = 2.0 * nv->getFrameStamp()->getSimulationTime();
        //float sine = sinf( angle );        // -1 -> 1
        //float v01 = 0.5f * sine + 0.5f;        //  0 -> 1
        //float v10 = 1.0f - v01;                //  1 -> 0

        double  fractpart, intpart;
        fractpart = modf (nv->getFrameStamp()->getSimulationTime() / 100.0f , &intpart);

        uniform->set( osg::Vec4(/*1.505f*/1.5f, /*0.8f*v01*/1.5f, 1.5f, 100*fractpart) ); 
    }
};


class texturesHolder  : public texturesHolder_base
{
public:
    struct textures_t
    {
        osg::ref_ptr<osg::Texture2D>      colorTex;
        osg::ref_ptr<osg::Texture2D>      normalTex;
        osg::ref_ptr<osg::Texture2D>      nightTex;
        osg::ref_ptr<osg::Texture2D>      detailsTex;
        osg::ref_ptr<osg::TextureCubeMap> envTex;
        osg::ref_ptr<osg::Texture2D>      decalTex;
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
            || mat_name.find("default") !=std::string::npos
            )
        {
            return texCreator(mats,mat_name,model_name); 
        }

        return texCreator(mats,"default",model_name);
    }

    osg::ref_ptr<osg::TextureCubeMap>   getEnvTexture()
    {
        return   envTex;
    }

    osg::ref_ptr<osg::Texture2D>   getDecalTexture()
    {
        return   decalTex;
    }

    friend texturesHolder_base&   getTextureHolder();

private:

    osg::ref_ptr<osg::Texture2D>      detailsTex;
    osg::ref_ptr<osg::Texture2D>      emptyTex;
    osg::ref_ptr<osg::TextureCubeMap> envTex;
    osg::ref_ptr<osg::Texture2D>      decalTex;

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
        envTex->setNumMipmapLevels(3);
        envTex->setUseHardwareMipMapGeneration(true);

        decalTex = new osg::Texture2D;
        decalTex->setTextureSize(1024, 1024);
        decalTex->setInternalFormat( GL_RGBA );
        decalTex->setBorderWidth( 0 );
        decalTex->setFilter( osg::Texture::MIN_FILTER, osg::Texture::NEAREST );
        decalTex->setFilter( osg::Texture::MAG_FILTER, osg::Texture::NEAREST );
#if 0
#if 1
        envTex->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP);
        envTex->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP);
        envTex->setWrap(osg::Texture::WRAP_R, osg::Texture::CLAMP);
        envTex->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR_MIPMAP_LINEAR);
        envTex->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);    
        // assign the six images to the texture object
        envTex->setImage(osg::TextureCubeMap::POSITIVE_X, osgDB::readImageFile("day_posx.jpg"));
        envTex->setImage(osg::TextureCubeMap::NEGATIVE_X, osgDB::readImageFile("day_negx.jpg"));
        envTex->setImage(osg::TextureCubeMap::POSITIVE_Y, osgDB::readImageFile("day_posy.jpg"));
        envTex->setImage(osg::TextureCubeMap::NEGATIVE_Y, osgDB::readImageFile("day_negy.jpg"));
        envTex->setImage(osg::TextureCubeMap::POSITIVE_Z, osgDB::readImageFile("day_posz.jpg"));
        envTex->setImage(osg::TextureCubeMap::NEGATIVE_Z, osgDB::readImageFile("day_negz.jpg"));
        // envTex->setUseHardwareMipMapGeneration(true);
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
            textures_t  t; 
            t.colorTex = new osg::Texture2D;
            t.nightTex = new osg::Texture2D;
            t.normalTex  = new osg::Texture2D;
            t.detailsTex = th.detailsTex;
            //t.detailsTex = new osg::Texture2D;

            //t.detailsTex->setImage( osgDB::readImageFile("Detail.dds",new osgDB::Options("")) );  
            //t.detailsTex->setWrap(  osg::Texture::WRAP_S, osg::Texture::REPEAT );
            //t.detailsTex->setWrap(  osg::Texture::WRAP_T, osg::Texture::REPEAT );


            t.envTex = th.envTex;

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
                    t.colorTex->setWrap(  osg::Texture::WRAP_S, osg::Texture::REPEAT );
                    t.colorTex->setWrap(  osg::Texture::WRAP_T, osg::Texture::REPEAT );
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
                    t.normalTex->setWrap(  osg::Texture::WRAP_S, osg::Texture::CLAMP );
                    t.normalTex->setWrap(  osg::Texture::WRAP_T, osg::Texture::CLAMP );
                    t.normalTex->setFilter(osg::Texture::MIN_FILTER,osg::Texture::LINEAR_MIPMAP_LINEAR);
                    t.normalTex->setMaxAnisotropy(16.0f);

                    normal_tex = true;
                } 
                else 	
                if(it->second.unit == 2)
                {
                    t.nightTex->setImage( osgDB::readImageFile(name) );
                    t.nightTex->setWrap(  osg::Texture::WRAP_S, osg::Texture::REPEAT );
                    t.nightTex->setWrap(  osg::Texture::WRAP_T, osg::Texture::REPEAT );
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

class programsHolder: public programsHolder_base
{
    //public:
    //    struct program_t
    //    {
    //        osg::ref_ptr<osg::Program> program;
    //    };

public:
    static inline const program_t& Create(std::string mat_name)
    {
        if(GetPrograms().find(mat_name)==GetPrograms().end())
        {
            program_t p;
            p.program = new osg::Program;
            p.program->setName(mat_name);
            if(GetShader(shaders::VS,mat_name))
            {
                auto vs = new osg::Shader( osg::Shader::VERTEX,  GetShader(shaders::VS,mat_name));
                p.program->addShader( vs );

            }
            //std::string fs_shader(GetShader(shaders::FS,mat_name));
            //fs_shader.replace('','');
            if(GetShader(shaders::FS,mat_name))
            {
                auto fs = new osg::Shader(osg::Shader::FRAGMENT, GetShader(shaders::FS,mat_name));
                p.program->addShader( fs );
            }

            p.program->addBindAttribLocation( "tangent" , 6 );
            p.program->addBindAttribLocation( "binormal", 7 );

            GetPrograms()[mat_name]=p;
        }

        return GetPrograms()[mat_name];
    }

private:

    static inline const char* GetShader(shaders::shader_t t, std::string mat_name)
    {
        if (mat_name.find("building") !=std::string::npos)
        {
            return shaders::building_mat::get_shader(t); 
        }
        else
            if (mat_name.find("tree") !=std::string::npos)
            {
                return shaders::tree_mat::get_shader(t);  
            }
            else
                if (mat_name.find("ground") !=std::string::npos || mat_name.find("sea") !=std::string::npos || mat_name.find("mountain") !=std::string::npos)
                {
                    return shaders::ground_mat::get_shader(t);  
                }    
                else
                    if (mat_name.find("concrete") !=std::string::npos)
                    {
                        return shaders::concrete_mat::get_shader(t);  
                    }
                    else
                        if (mat_name.find("railing") !=std::string::npos)
                        {
                            return shaders::railing_mat::get_shader(t);  
                        }
                        else
                            if (mat_name.find("plane") !=std::string::npos)
                            {
                                return shaders::plane_mat::get_shader(t);  
                            }
                            else
                                if (mat_name.find("panorama") !=std::string::npos)
                                {
                                    return shaders::panorama_mat::get_shader(t);  
                                }
                                else
                                    if (mat_name.find("sky") !=std::string::npos)
                                    {
                                        return shaders::sky_fog_mat::get_shader(t);  
                                    }            
                                    else
                                        if (mat_name.find("clouds") !=std::string::npos)
                                        {
                                            return shaders::clouds_mat::get_shader(t);  
                                        }
                                        else
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

    stateset->addUniform( new osg::Uniform("colorTex"      , 0) );
    stateset->addUniform( new osg::Uniform("normalTex"     , 1) ); 
    stateset->addUniform( new osg::Uniform("nightTex"      , 2) );
    stateset->addUniform( new osg::Uniform("detailTex"     , 3) ); 
    stateset->addUniform( new osg::Uniform("envTex"        , 4) ); 
    stateset->addUniform( new osg::Uniform("ViewDecalMap"  , 5) );
    stateset->addUniform( new osg::Uniform("shadowTexture0", BASE_SHADOW_TEXTURE_UNIT) );


    stateset->setAttributeAndModes( p.program.get() );

    if ( mat_name.find("panorama") !=std::string::npos )
    {     
        osg::Uniform* uni_fog =  new osg::Uniform("fog_params", osg::Vec4(0.0f, 0.0f, 0.0f, 0.0f));
        stateset->addUniform( uni_fog );
        uni_fog->setUpdateCallback(new FogCallback);
        uni_fog->setDataVariance(osg::Object::DYNAMIC);
    }

    if (   mat_name.find("panorama") !=std::string::npos
        || mat_name.find("railing")  !=std::string::npos 
        || mat_name.find("tree")     !=std::string::npos 
        )
    { 
        stateset->setMode(GL_SAMPLE_ALPHA_TO_COVERAGE,osg::StateAttribute::ON);               
    }

    osg::StateAttribute::GLModeValue value = osg::StateAttribute::ON|osg::StateAttribute::OVERRIDE;
    stateset->setTextureAttributeAndModes( 0, t.colorTex.get(), value );
    stateset->setTextureAttributeAndModes( 1, t.normalTex.get(), value );
    stateset->setTextureAttributeAndModes( 2, t.nightTex.get(), value );

    stateset->setTextureAttributeAndModes( 3, t.detailsTex.get(), value );
    stateset->setTextureAttributeAndModes( 4, t.envTex.get(), value );
    stateset->setTextureAttributeAndModes( 5, getTextureHolder().getDecalTexture().get()/*GetShadowMap()->getTexture()*/, value ); 
    stateset->setMode(GL_TEXTURE_CUBE_MAP_SEAMLESS_ARB, osg::StateAttribute::ON); 
}

} //namespace creators 


namespace mat
{


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
                    mats_.insert(materials_t::value_type(std::string(m.attribute("name").as_string()),tex));
                }
            }	

        }
        else
            std::cerr << "File not found: " << in_file_name;

        return mats_;
    }

}
