
#include "stdafx.h"
#include "dom/domcommon_newparam_type.h"

namespace fs = boost::filesystem;                  boost::filesystem::path& dataPath();
boost::filesystem::path& tmpPath();

typedef std::basic_string<TCHAR>   tstring;

//osg::Texture::WrapMode getWrapMode(domFx_sampler_wrap_common domWrap)
//{
//    switch (domWrap)
//    {
//    case FX_SAMPLER_WRAP_COMMON_WRAP:
//        return osg::Texture::REPEAT;
//    case FX_SAMPLER_WRAP_COMMON_MIRROR:
//        return osg::Texture::MIRROR;
//    case FX_SAMPLER_WRAP_COMMON_CLAMP:
//        return osg::Texture::CLAMP_TO_EDGE;
//    case FX_SAMPLER_WRAP_COMMON_NONE:
//    case FX_SAMPLER_WRAP_COMMON_BORDER:
//        return osg::Texture::CLAMP_TO_BORDER;
//    default:
//        OSG_WARN << "Unrecognised domFx_sampler_wrap_common." << std::endl;
//    }
//    return osg::Texture::CLAMP;
//}

//osg::Texture::FilterMode getFilterMode(domFx_sampler_filter_common domFilter, bool allowMipMap)
//{
//    switch (domFilter)
//    {
//    case FX_SAMPLER_FILTER_COMMON_NONE:
//    case FX_SAMPLER_FILTER_COMMON_NEAREST:
//        return osg::Texture::NEAREST;
//    case FX_SAMPLER_FILTER_COMMON_LINEAR:
//        return osg::Texture::LINEAR;
//    case FX_SAMPLER_FILTER_COMMON_NEAREST_MIPMAP_NEAREST:
//        return allowMipMap ? osg::Texture::NEAREST_MIPMAP_NEAREST : osg::Texture::NEAREST;
//    case FX_SAMPLER_FILTER_COMMON_LINEAR_MIPMAP_NEAREST:
//        return allowMipMap ? osg::Texture::LINEAR_MIPMAP_NEAREST : osg::Texture::LINEAR;
//    case FX_SAMPLER_FILTER_COMMON_NEAREST_MIPMAP_LINEAR:
//        return allowMipMap ? osg::Texture::NEAREST_MIPMAP_LINEAR : osg::Texture::NEAREST;
//    case FX_SAMPLER_FILTER_COMMON_LINEAR_MIPMAP_LINEAR:
//        return allowMipMap ? osg::Texture::LINEAR_MIPMAP_LINEAR : osg::Texture::LINEAR;
//    default:
//        OSG_WARN << "Unrecognised domFx_sampler_filter_common." << std::endl;
//    }
//    return osg::Texture::LINEAR;
//}

class TextureParameters
{
public:
    TextureParameters()
        : wrap_s(FX_SAMPLER_WRAP_COMMON_WRAP), wrap_t(FX_SAMPLER_WRAP_COMMON_WRAP),
        filter_min(FX_SAMPLER_FILTER_COMMON_LINEAR_MIPMAP_LINEAR), filter_mag(FX_SAMPLER_FILTER_COMMON_LINEAR_MIPMAP_LINEAR),
        transparent(false), opaque(FX_OPAQUE_ENUM_A_ONE), transparency(1.0f)
    {}

	// FIXME have no < for domFloat4
    //bool operator < (const TextureParameters& rhs) const
    //{
    //    int diffStr = filename.compare(rhs.filename);
    //    if (diffStr) return diffStr < 0;
    //    if (wrap_s != rhs.wrap_s) return wrap_s < rhs.wrap_s;
    //    if (wrap_t != rhs.wrap_t) return wrap_t < rhs.wrap_t;
    //    if (filter_min != rhs.filter_min) return filter_min < rhs.filter_min;
    //    if (filter_mag != rhs.filter_mag) return filter_mag < rhs.filter_mag;
    //    if (transparency != rhs.transparency) return transparency < rhs.transparency;
    //    if (opaque != rhs.opaque) return opaque < rhs.opaque;
    //    if (transparent != rhs.transparent) return transparent < rhs.transparent;
    //    return border < rhs.border;
    //}

    std::string filename;
    domFx_sampler_wrap_common   wrap_s, wrap_t;
    domFx_sampler_filter_common filter_min, filter_mag;
    domFloat4 border;

    //The following parameters are for transparency textures, to handle
    //COLLADA's horrible transparency spec.
    bool transparent;
    domFx_opaque_enum opaque;
    float transparency;
};

fs::path& dataPath() {
	static fs::path dataPath_;
	return dataPath_;
}

fs::path& tmpPath() {
	static fs::path tmpPath_;
	return tmpPath_;
}

// TODO mat_reader mat_writer

struct texture_t
{
	int         unit;
	std::string path;
};

typedef std::multimap<std::string,texture_t> mat_profiles_t;

struct mat_reader
{
	mat_reader()
	{
    }

	mat_reader(std::string full_path)
	{
		mats_ = read (full_path);
	}

	static mat_profiles_t  read (std::string full_path)
	{
		pugi::xml_document doc;
		mat_profiles_t mats_;

		fs::path fs_path(full_path);

		std::string in_file_name = fs_path.parent_path().string() + '/' + fs_path.stem()/*leaf()*/.string() + ".mat.xml";
		
		bool l = doc.load_file(in_file_name.c_str());
		
		pugi::xml_node root = doc.child("root");

		for (pugi::xml_node m = root.first_child(); m; m = m.next_sibling())
		{
			for (pugi::xml_node t = m.first_child(); t; t = t.next_sibling())
			{
				texture_t tex;
				tex.path = t.attribute("path").as_string();
				tex.unit = t.attribute("unit").as_int();
				mats_.insert(mat_profiles_t::value_type(std::string(m.attribute("name").as_string()),tex));
			}
		}	

		return mats_;
	}

private:
	mat_profiles_t mats_;
};

struct xml_helper
{
	xml_helper(std::string full_path)
	{
		assert(outFileName().empty());
		fs::path fs_path(full_path);
		outFileName(fs_path.parent_path().string() + '/' + fs_path.stem()/*leaf()*/.string() + ".mat.xml");
	}

	static pugi::xml_node& createNewMaterial(std::string name)
	{
		xml_helper::currentMaterial() = xml_helper::getRoot().append_child("Material");
		xml_helper::currentMaterial().append_attribute("name") = name.c_str();
		return xml_helper::currentMaterial();
    }

	static inline pugi::xml_node& currentMaterial() {
		return currentMaterial(nullptr);
	}

private:

	static inline std::string outFileName(std::string file_name=std::string()) {
		static std::string out_file_name(file_name);
		if(!file_name.empty()) out_file_name = file_name;
		return out_file_name;
	}

	static inline pugi::xml_node& getRoot() {
		static root_wrap rw;
		return rw.root_node;
	}
	
	static pugi::xml_document& getDoc() {
		static pugi::xml_document doc;
		return doc;
	}

	static inline pugi::xml_node& currentMaterial(pugi::xml_node* n) {
		static pugi::xml_node node;
		if(n != nullptr) node = *n;
		return node;
	}

	struct root_wrap
	{
		root_wrap()
			: root_node(getDoc().append_child("root"))
		{
			root_node.append_attribute("xmlns:xi") = "http://www.w3.org/2001/XInclude";
			auto xi_node = root_node.append_child("xi:include");
			xi_node.append_attribute("href") = "materials_ext.xml";
		}

		~root_wrap()
		{ getDoc().save_file(outFileName().c_str());}

		pugi::xml_node root_node;
	};


};

DAE& GetDAE()
{	
	static DAE dae;
	return dae;
}



domEffect* currentEffect(domEffect* de = nullptr)
{
	static domEffect* _currentEffect;
	if(de != nullptr) _currentEffect = de;
	return _currentEffect;
}

std::string sourceFilePath(const tstring& fileName) {
	return (dataPath() / fileName).string();// native_file_string();
}

inline daeElement *getElementFromURI( daeURI &uri )
{
    if ( uri.getState() == daeURI::uri_loaded || uri.getState() == daeURI::uri_pending ) {
        uri.resolveElement();
    }
    return uri.getElement();
}

inline daeElement *getElementFromIDRef( daeIDRef &idref )
{
    if ( idref.getState() == daeIDRef::id_loaded || idref.getState() == daeIDRef::id_pending ) {
        idref.resolveElement();
    }
    return idref.getElement();
}

std::string /*ReaderWriterDAE::*/ConvertColladaCompatibleURIToFilePath(const std::string& uri)
{
    std::string path(uri);
	// Reciprocal of ConvertFilePathToColladaCompatibleURI()
//#ifdef OSG_USE_UTF8_FILENAME
//    std::string path( cdom::uriToNativePath( uri ) );
//#else
//    std::string path( osgDB::convertStringFromCurrentCodePageToUTF8( cdom::uriToNativePath(uri) ) );
//#endif
//    replace(path, "%23", "#");
    return path;
}

std::string processImagePath(const domImage* pDomImage) /*const*/
{
    if (pDomImage == NULL)
    {
        std::cout << "Could not locate image for texture" << std::endl;
        return std::string();
    }

    //Got a sampler and a surface and an imaged. Time to create the texture stuff for osg
    if (pDomImage->getInit_from())
    {
        pDomImage->getInit_from()->getValue().validate();
        if (strcmp(pDomImage->getInit_from()->getValue().getProtocol(), "file") == 0)
        {
            // FIXME maybe we need full path as option
			//std::string path = pDomImage->getInit_from()->getValue().pathDir() +
            //                   pDomImage->getInit_from()->getValue().pathFile();
            // path = /*ReaderWriterDAE::*/ConvertColladaCompatibleURIToFilePath(path);
			std::string path = pDomImage->getInit_from()->getValue().pathFile();
            if (path.empty())
            {
                std::cout << "Unable to get path from URI." << std::endl;
                return std::string();
            }
#ifdef WIN32
            // If the path has a drive specifier or a UNC name then strip the leading /
            if (path.size() > 2 && (path[2] == ':' || (path[1] == '/' && path[2] == '/')))
                return path.substr(1, std::string::npos);
#endif
            return path;
        }
        else
        {
            std::cout << "Only images with a \"file\" scheme URI are supported in this version." << std::endl;
        }
    }
    else
    {
        std::cout << "Embedded image data is not supported in this version." << std::endl;
    }
    return std::string();
}

std::shared_ptr<TextureParameters> processTexture(
    domCommon_color_or_texture_type_complexType::domTexture *tex
)
{
    std::shared_ptr<TextureParameters> parameters(new TextureParameters);
    //parameters.transparent = tuu == TRANSPARENCY_MAP_UNIT;
    //parameters.opaque = opaque;
    //parameters.transparency = transparency;

    //find the newparam for the sampler based on the texture attribute
    domFx_sampler2D_common *sampler = NULL;
    domFx_surface_common *surface = NULL;
    domImage *dImg = NULL;

    std::string target = std::string("./") + std::string(tex->getTexture());
    // OSG_INFO<<"processTexture("<<target<<")"<<std::endl;

    daeSIDResolver res1( /*_currentEffect*/currentEffect(), target.c_str() );
    daeElement *el = res1.getElement();

    if (el == NULL )
    {
        std::cout << "Could not locate newparam for texture sampler2D \"" << tex->getTexture() <<
            "\". Checking if data does incorrect linking straight to the image" << std::endl;
        GetDAE().getDatabase()->getElement( (daeElement**)&dImg, 0, tex->getTexture(), "image" );
        if (dImg != NULL )
        {
            std::cout << "Direct image link found. Data is incorrect but will continue to load texture" << std::endl;
        }
    }
    else
    {
        domCommon_newparam_type *cnp = daeSafeCast< domCommon_newparam_type >( el );
        domFx_newparam_common *npc = daeSafeCast< domFx_newparam_common >( el );

        if (cnp != NULL )
        {
            sampler = cnp->getSampler2D();
        }
        else if (npc != NULL )
        {
            sampler = npc->getFx_basic_type_common()->getSampler2D();
        }

        if (sampler == NULL )
        {
            std::cout << "Wrong newparam type. Expected sampler2D" << std::endl;
            return nullptr;
        }

        if (sampler->getSource() == NULL )
        {
            std::cout << "Could not locate source for sampler2D" << std::endl;
            return nullptr;
        }

        //find the newparam for the surface based on the sampler2D->source value
        target = std::string("./") + std::string( sampler->getSource()->getValue() );
        daeSIDResolver res2( /*_currentEffect*/currentEffect(), target.c_str() );
        el = res2.getElement();
        if (el == NULL )
        {
            std::cout << "Could not locate newparam for source " << sampler->getSource()->getValue() << std::endl;
            return nullptr;
        }
        cnp = daeSafeCast< domCommon_newparam_type >( el );
        npc = daeSafeCast< domFx_newparam_common >( el );

        if (cnp != NULL )
        {
            surface = cnp->getSurface();
        }
        else if (npc != NULL )
        {
            surface = npc->getFx_basic_type_common()->getSurface();
        }

        if (surface == NULL )
        {
            std::cout << "Wrong newparam type. Expected surface" << std::endl;
            return NULL;
        }

        //look for the domImage based on the surface initialization stuff
        daeIDRef &ref = surface->getFx_surface_init_common()->getInit_from_array()[0]->getValue();
        dImg = daeSafeCast< domImage >( getElementFromIDRef( ref ) );
    }

    parameters->filename = processImagePath(dImg);
	std::cout << "ImagePath: " << parameters->filename << std::endl;
    if (parameters->filename.empty())
    {
		
        return NULL;
    }
	

    //set texture parameters
    if (sampler)
    {
        if (sampler->getWrap_s())
        {
            parameters->wrap_s = sampler->getWrap_s()->getValue();//getWrapMode(sampler->getWrap_s()->getValue());
        }
        if (sampler->getWrap_t())
        {
            parameters->wrap_t = sampler->getWrap_t()->getValue();//getWrapMode(sampler->getWrap_s()->getValue());
        }

        if (sampler->getMinfilter() && sampler->getMinfilter()->getValue() != FX_SAMPLER_FILTER_COMMON_NONE)
        {
            parameters->filter_min = sampler->getMinfilter()->getValue();//getFilterMode(sampler->getMinfilter()->getValue(), true);
        }
        if (sampler->getMagfilter() && sampler->getMagfilter()->getValue() != FX_SAMPLER_FILTER_COMMON_NONE)
        {
            parameters->filter_mag = sampler->getMagfilter()->getValue();//getFilterMode(sampler->getMagfilter()->getValue(), false);
        }

        if (sampler->getBorder_color() != NULL )
        {
            const domFloat4& col = sampler->getBorder_color()->getValue();
            parameters->border = col;// parameters.border.set(col[0], col[1], col[2], col[3]);
        }
    }

	   
	//osg::Texture2D* t2D = NULL;
    //TextureParametersMap::const_iterator mapIt = _textureParamMap.find(parameters);
    //if (mapIt != _textureParamMap.end())
    //{
    //    t2D = mapIt->second.get();
    //}
    //else
    //{
    //    osg::ref_ptr<osg::Image> img = osgDB::readRefImageFile(parameters.filename);

    //    if (!img.valid())
    //    {
    //        _textureParamMap[parameters] = NULL;
    //        return NULL;
    //    }

    //    OSG_INFO<<"  processTexture(..) - readImage("<<parameters.filename<<")"<<std::endl;

    //    if (tuu == TRANSPARENCY_MAP_UNIT)
    //    {
    //        img = processImageTransparency(img.get(), opaque, transparency);
    //    }

    //    t2D = new osg::Texture2D(img.get());

    //    t2D->setWrap( osg::Texture::WRAP_S, parameters.wrap_s);
    //    t2D->setWrap( osg::Texture::WRAP_T, parameters.wrap_t);
    //    t2D->setFilter( osg::Texture::MIN_FILTER, parameters.filter_min);
    //    t2D->setFilter( osg::Texture::MAG_FILTER, parameters.filter_mag);
    //    t2D->setBorderColor(parameters.border);

    //    _textureParamMap[parameters] = t2D;
    //}

    //_texCoordSetMap[TextureToCoordSetMap::key_type(ss, tuu)] = tex->getTexcoord();

    return /*t2D*/parameters;
}

enum  mat_type_t {DIFFUSE,BUMPMAP,AMBIENT,EMISSION,SPECULAR};

inline int stringify(mat_type_t mt)
{
	if     (mt == DIFFUSE ) return 0;
    else if(mt == BUMPMAP ) return 1;
	else if(mt == EMISSION) return 2;
    else if(mt == AMBIENT ) return 3;
    else if(mt == SPECULAR) return 4;
	return -1;
}

inline std::string stringify(domFx_sampler_wrap_common domWrap)
{
    switch (domWrap)
    {
    case FX_SAMPLER_WRAP_COMMON_WRAP:
        return "repeat";
    case FX_SAMPLER_WRAP_COMMON_MIRROR:
        return "mirror";
    case FX_SAMPLER_WRAP_COMMON_CLAMP:
        return "clamp_to_edge";
    case FX_SAMPLER_WRAP_COMMON_NONE:
    case FX_SAMPLER_WRAP_COMMON_BORDER:
        return "clamp_to_border";
    default:
        std::cout << "Unrecognised domFx_sampler_wrap_common." << std::endl;
    }
    return "clamp";
}

bool processColorOrTextureType( domCommon_color_or_texture_type *cot, mat_type_t mt )
{
    if (cot == NULL )
    {
        return false;
    }
    bool retVal = false;

    std::string texCoordSet;
	std::shared_ptr<TextureParameters> p = nullptr;
    if (cot->getTexture() != NULL)
    {
		p = processTexture( cot->getTexture());
        retVal = true;
    }	

	if(p){
		auto tex = xml_helper::currentMaterial().append_child("texture");
		tex.append_attribute("unit") = stringify(mt);
		tex.append_attribute("path") = p->filename.c_str();
		tex.append_attribute("wrap_s") = stringify(p->wrap_s).c_str();
		tex.append_attribute("wrap_t") = stringify(p->wrap_t).c_str();
	}

    //TODO: Make all channels process <param ref=""> type of value
    //if (channel == osg::Material::EMISSION )
    //{
    //    if (cot->getColor() != NULL )
    //    {
    //        domFloat4 &f4 = cot->getColor()->getValue();
    //        mat->setEmission( osg::Material::FRONT_AND_BACK, osg::Vec4( f4[0], f4[1], f4[2], (f4.getCount()==4)? f4[3] : 1.0 ) );
    //        retVal = true;
    //    }
    //    else if (cot->getParam() != NULL)
    //    {
    //        domFloat4 f4;
    //        if (GetFloat4Param(cot->getParam()->getRef(), f4))
    //        {
    //            mat->setEmission( osg::Material::FRONT_AND_BACK, osg::Vec4( f4[0], f4[1], f4[2], f4[3] ) );
    //            retVal = true;
    //        }
    //    }
    //    else if (cot->getTexture() != NULL)
    //    {
    //        if (sa != NULL)
    //        {
    //            *sa = processTexture( cot->getTexture(), ss, MAIN_TEXTURE_UNIT);
    //            retVal = true;
    //        }
    //        else
    //            OSG_WARN << "Currently no support for <texture> in Emission channel " << std::endl;
    //    }
    //    else
    //    {
    //        OSG_WARN << "Missing <color>, <param> or <texture> in Emission channel " << std::endl;
    //    }
    //}
    //else if (channel == osg::Material::AMBIENT )
    //{
    //    if (cot->getColor() != NULL )
    //    {
    //        domFloat4 &f4 = cot->getColor()->getValue();
    //        mat->setAmbient( osg::Material::FRONT_AND_BACK, osg::Vec4( f4[0], f4[1], f4[2], (f4.getCount()==4)? f4[3] : 1.0 ) );
    //        retVal = true;
    //    }
    //    else if (cot->getParam() != NULL)
    //    {
    //        domFloat4 f4;
    //        if (cot->getParam()->getRef() != 0 && GetFloat4Param(cot->getParam()->getRef(), f4))
    //        {
    //            mat->setAmbient( osg::Material::FRONT_AND_BACK, osg::Vec4( f4[0], f4[1], f4[2], f4[3] ) );
    //            retVal = true;
    //        }
    //    }
    //    else if (cot->getTexture() != NULL)
    //    {
    //        if (sa != NULL)
    //            *sa = processTexture( cot->getTexture(), ss, AMBIENT_OCCLUSION_UNIT);
    //        else
    //        {
    //            OSG_WARN << "Currently no support for <texture> in Ambient channel " << std::endl;
    //            mat->setAmbient( osg::Material::FRONT_AND_BACK, osg::Vec4( 0.2f, 0.2f, 0.2f, 1.0f ) );
    //        }
    //        retVal = true;
    //   }
    //    else
    //    {
    //        OSG_WARN << "Missing <color>, <param> or <texture> in Ambient channel " << std::endl;
    //    }
    //}
    //else if (channel == osg::Material::DIFFUSE )
    //{
    //    if (cot->getColor() != NULL)
    //    {
    //        domFloat4 &f4 = cot->getColor()->getValue();
    //        mat->setDiffuse( osg::Material::FRONT_AND_BACK, osg::Vec4( f4[0], f4[1], f4[2], (f4.getCount()==4)? f4[3] : 1.0 ) );
    //        retVal = true;
    //    }
    //    else if (cot->getTexture() != NULL)
    //    {
    //        if (sa != NULL)
    //            *sa = processTexture( cot->getTexture(), ss, MAIN_TEXTURE_UNIT);
    //        else
    //        {
    //            OSG_WARN << "Currently no support for <texture> in Diffuse channel " << std::endl;
    //            mat->setDiffuse( osg::Material::FRONT_AND_BACK, osg::Vec4( 0.8f, 0.8f, 0.8f, 1.0f ) );
    //        }
    //        domExtra *extra = cot->getTexture()->getExtra();
    //        if (extra != NULL && extra->getType() != NULL && strcmp( extra->getType(), "color" ) == 0 )
    //        {
    //            //the extra data for osg. Diffuse color can happen with a texture.
    //            for ( unsigned int i = 0; i < extra->getTechnique_array().getCount(); i++ )
    //            {
    //                domTechnique *teq = extra->getTechnique_array()[i];
    //                if (strcmp( teq->getProfile(), "SCEI" ) == 0 )
    //                {
    //                    osg::Vec4 col;
    //                    domAny *dcol = (domAny*)(daeElement*)teq->getContents()[0];
    //                    std::istringstream diffuse_colour((const char *)dcol->getValue());
    //                    diffuse_colour >> col.r() >> col.g() >> col.b() >> col.a();
    //                    mat->setDiffuse( osg::Material::FRONT_AND_BACK, col );
    //                    break;
    //                }
    //            }
    //        }
    //        retVal = true;
    //    }
    //    else if (cot->getParam() != NULL)
    //    {
    //        domFloat4 f4;
    //        if (GetFloat4Param(cot->getParam()->getRef(), f4))
    //        {
    //            mat->setDiffuse( osg::Material::FRONT_AND_BACK, osg::Vec4( f4[0], f4[1], f4[2], f4[3] ) );
    //            retVal = true;
    //        }
    //    }
    //    else
    //    {
    //        OSG_WARN << "Missing <color>, <param> or <texture> in Diffuse channel " << std::endl;
    //    }
    //}
    //else if (channel == osg::Material::SPECULAR )
    //{
    //    if (cot->getColor() != NULL )
    //    {
    //        domFloat4 &f4 = cot->getColor()->getValue();
    //        mat->setSpecular( osg::Material::FRONT_AND_BACK, osg::Vec4( f4[0], f4[1], f4[2], (f4.getCount()==4)? f4[3] : 1.0 ) );
    //        retVal = true;
    //    }
    //    else if (cot->getParam() != NULL)
    //    {
    //        domFloat4 f4;
    //        if (GetFloat4Param(cot->getParam()->getRef(), f4))
    //        {
    //            mat->setSpecular( osg::Material::FRONT_AND_BACK, osg::Vec4( f4[0], f4[1], f4[2], f4[3] ) );
    //            retVal = true;
    //        }
    //    }
    //    else if (cot->getTexture() != NULL)
    //    {
    //        OSG_WARN << "Currently no support for <texture> in Specular channel " << std::endl;
    //    }
    //    else
    //    {
    //        OSG_WARN << "Missing <color>, <param> or <texture> in Specular channel " << std::endl;
    //    }

    //    if (fop != NULL && fop->getFloat() != NULL )
    //    {
    //        float shininess = fop->getFloat()->getValue();
    //        if (blinn)
    //        {
    //            // If the blinn mode is in the range [0,1] rescale it to [0,128]
    //            if (shininess < 1)
    //                shininess *= 128.0f;
    //        }
    //        mat->setShininess( osg::Material::FRONT_AND_BACK, shininess );
    //    }
    //}

    return retVal;
}

std::string imagePathFromSampler(domCOLLADA& root, const std::string& sampler_name)
{
    domLibrary_effects* e = daeSafeCast<domLibrary_effects>(root.getDescendant(COLLADA_ELEMENT_LIBRARY_EFFECTS));

    std::string image_name;

    if(e)
    {
        domEffect_Array& ea =  e->getEffect_array();
        for (size_t i = 0; i < ea.getCount(); i++)
        {	
            xsID s = ea[i]->getId();

            domFx_profile_abstract_Array & aaa =  ea[i]->getFx_profile_abstract_array();
            FIXME(Опасное упрощение)
                domProfile_COMMON *pc = daeSafeCast< domProfile_COMMON >( aaa[0] );

            //const domImage_Array& ia =  pc->getImage_array();
            //for (size_t j = 0; j < ia.getCount(); j++)
            //{	
            //    xsNCName s = ia[j]->getName();
            //    std::string path = ia[j]->getInit_from()->getValue().pathFile();

            //    int jj = 1;
            //}


            const domCommon_newparam_type_Array & npa =  pc->getNewparam_array();

            std::string surface_name;

            for (size_t j = 0; j < npa.getCount(); j++)
            {	
                xsNCName sid = npa[j]->getSid();

                domFx_sampler2D_commonRef sampl = npa[j]->getSampler2D();

                if(sampl && sampler_name==sid)
                { 
                    const domFx_sampler2D_common_complexType::domSourceRef src = sampl->getSource();
                    surface_name = src->getValue();
                    break;
                }
            }

            for (size_t j = 0; j < npa.getCount(); j++)
            {
                xsNCName sid = npa[j]->getSid();
                domFx_surface_commonRef  sur = npa[j]->getSurface();

                if(sur && surface_name==sid)
                { 
                    domFx_surface_init_commonRef sir = sur->getFx_surface_init_common();
                    const domFx_surface_init_from_common_Array & aaaa = sir->getInit_from_array();
                    FIXME(Опасное упрощение)
                        for (size_t jj = 0; jj < aaaa.getCount(); jj++)
                        {
                            const xsIDREF & vv = aaaa[jj]->getValue();
                            image_name = vv.getID();
                            break;
                        }

                }

            }

        }
    }

    domLibrary_images* il = daeSafeCast<domLibrary_images>(root.getDescendant(COLLADA_ELEMENT_LIBRARY_IMAGES));

    if(il)
    {
        domImage_Array& ia =  il->getImage_array();
        for (size_t i = 0; i < ia.getCount(); i++)
        {	
            xsNCName s = ia[i]->getName();
            if(image_name == s)
                return ia[i]->getInit_from()->getValue().pathFile();
        }
    }

    return "";
}


template <typename T> 
void process(T* p)
{
    if (p != NULL )
    {
        std::cout << "Emission: " <<  std::endl;
        processColorOrTextureType(p->getEmission(),EMISSION );
        std::cout << "Ambient: " <<  std::endl;
        processColorOrTextureType(p->getAmbient(),AMBIENT );
        std::cout << "Diffuse: " <<  std::endl;
        processColorOrTextureType(p->getDiffuse(),DIFFUSE );
	    std::cout << "Specular: " <<  std::endl;
        processColorOrTextureType( p->getSpecular(),SPECULAR);
    }
}

template <> 
void process(domProfile_COMMON::domTechnique::domConstant* p)
{
    if (p != NULL )
    {
        std::cout << "Emission: " <<  std::endl;
        processColorOrTextureType(p->getEmission(),EMISSION );
    }
}

template <> 
void process(domProfile_COMMON::domTechnique::domLambert* p)
{
    if (p != NULL )
    {
        std::cout << "Emission: " <<  std::endl;
        processColorOrTextureType(p->getEmission(),EMISSION );
        std::cout << "Ambient: " <<  std::endl;
        processColorOrTextureType(p->getAmbient(), AMBIENT);
        std::cout << "Diffuse: " <<  std::endl;
        processColorOrTextureType(p->getDiffuse(),DIFFUSE );
    }
}

void processProfileCOMMON( domCOLLADA& root,domProfile_COMMON *pc )
{
    domProfile_COMMON::domTechnique *teq = pc->getTechnique();

    domProfile_COMMON::domTechnique::domConstant *c = teq->getConstant();
    domProfile_COMMON::domTechnique::domLambert *l = teq->getLambert();
    domProfile_COMMON::domTechnique::domPhong *p = teq->getPhong();
    domProfile_COMMON::domTechnique::domBlinn *b = teq->getBlinn();
    
    std::string name;
	for ( size_t i = 0; i < teq->getExtra_array().getCount(); i++ )
    {	
			domExtra *extra = daeSafeCast< domExtra >( teq->getExtra_array()[i] );
			auto arr = extra->getChildren();
			for ( size_t j = 0; j < arr.getCount(); j++ )
			{
				name = arr[j]->getElementName();
				auto bump = arr[j]->getChild("bump");
				auto bump_texture = bump->getChild("texture");
                
                std::string file_name = imagePathFromSampler(root,  bump_texture->getAttribute("texture"));
                
                if(!file_name.empty()){
                    auto tex = xml_helper::currentMaterial().append_child("texture");
                    tex.append_attribute("unit") = stringify(BUMPMAP);
                    tex.append_attribute("path") = file_name.c_str();
                    FIXME(И какой у нас wrap для bump?)
                    tex.append_attribute("wrap_s") = stringify(FX_SAMPLER_WRAP_COMMON_CLAMP).c_str();
                    tex.append_attribute("wrap_t") = stringify(FX_SAMPLER_WRAP_COMMON_CLAMP).c_str();
                }

				std::cout << "Bump: " << imagePathFromSampler(root,  bump_texture->getAttribute("texture")) <<  std::endl;
			}
	}

    process(c);
    process(l);
    process(p);
    process(b); 
	
}

void processEffect( domCOLLADA& root,domEffect *effect )
{
    bool hasCOMMON = false;

    for ( size_t i = 0; i < effect->getFx_profile_abstract_array().getCount(); i++ )
    {
        domProfile_COMMON *pc = daeSafeCast< domProfile_COMMON >( effect->getFx_profile_abstract_array()[i] );
        if (pc != NULL )
        {
            if (hasCOMMON )
            {
                std::cout << "Effect already has a profile_COMMON. Skipping this one" << std::endl;
                continue;
            }
            // _currentEffect = effect;
			currentEffect(effect);
            processProfileCOMMON(root,pc);
            hasCOMMON = true;
            continue;
        }

        std::cout << "unsupported effect profile " << effect->getFx_profile_abstract_array()[i]->getTypeName() << std::endl;
    }
}

void    processMaterial( domCOLLADA& root, domMaterial *mat )
{
    if (!mat)
    {
        return;
    }

    if (mat->getName()) {
		std::cout << "Material: " << mat->getName() << std::endl;
		//xml_helper::currentMaterial() = xml_helper::getRoot().append_child("Material");
		//xml_helper::currentMaterial().append_attribute("name") = mat->getName();
		xml_helper::createNewMaterial(mat->getName());
    }

    domInstance_effect * _currentInstance_effect = mat->getInstance_effect();

    if (!_currentInstance_effect)
    {
        return;
    }
    domEffect *effect = daeSafeCast< domEffect >( getElementFromURI( _currentInstance_effect->getUrl() ) );
    if (effect)
    {
        processEffect(root,effect);

        //TODO: process all of the setParams that could happen here in the material. ESP. the textures
    }
    else
    {
        std::cout << "Failed to locate effect " << mat->getInstance_effect()->getUrl().getURI() << std::endl;
    }
}



void processModel(const std::string& full_path) {

	//domVisual_scene* visualScene = daeSafeCast<domVisual_scene>(root.getDescendant("visual_scene"));

	//domNode_Array& nodes = visualScene->getNode_array();
	//for (size_t i = 0; i < nodes.getCount(); i++)
	//{	
	//	//lookup<Node, domNode>(*nodes[i]);
	//	xsNCName s = nodes[i]->getName();
	//}

    // mat_reader mt(full_path);
    
    domCOLLADA* root = GetDAE().open(full_path);
    xml_helper xw(full_path);

	domLibrary_materials* m = daeSafeCast<domLibrary_materials>(root->getDescendant(COLLADA_ELEMENT_LIBRARY_MATERIALS));
	if(m){
		
		domMaterial_Array& ms = m->getMaterial_array();
		for (size_t i = 0; i < ms.getCount(); i++)
		{	
			xsNCName s = ms[i]->getName();
			processMaterial( *root,ms[i] );
		}
	}
	
}



int _tmain(int argc, _TCHAR* argv[])
{
    if(argc!=2) {
        printf("Invalid args.\n");
        exit(1);
    }
   
    try {
        std::string file = sourceFilePath(tstring(argv[1]));
    	processModel(file);
	}
	catch (...) {
		 printf("Something went wrong.\n");
		 return 1;
	}

	return 0;
}

