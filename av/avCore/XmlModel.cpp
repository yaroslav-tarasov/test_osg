#include "stdafx.h"
#include "av/precompiled.h"
#include "av/avCore/Utils.h"
#include "av/avCore/XmlModel.h"

//
//  ext
//
#include "pugixml.hpp"


namespace avCore
{

    xml_model_t  ModelReader::Load (std::string full_path)
    {
        pugi::xml_document  doc;
        xml_model_t         data_;

        bool l = doc.load_file(full_path.c_str());
        if(l)
        {
            pugi::xml_node root = doc.child("root");
			
			pugi::xml_node MainModel =  root.child("MainModel");



			
            pugi::xml_node file_node = MainModel.child("File");
            if(file_node)
               data_.main_model  = file_node.attribute("path").as_string();
            else
               data_.main_model   = MainModel.attribute("path").as_string();
            
            std::string au_val;
            data_.lod3 = true;

            pugi::xml_node param_node = MainModel.child("Parameters");
            if(param_node)
            {
                data_.scale = param_node.attribute("scale").as_float(1.0);
                au_val      = param_node.attribute("axis_up").as_string("Z");
                data_.lod3  = param_node.attribute("lod3").as_string("on")!=std::string("off");
            }
            else
            {
                data_.scale = MainModel.attribute("scale").as_float(1.0);
                au_val      = MainModel.attribute("axis_up").as_string("Z");
            }


			for (pugi::xml_node pivot_node = MainModel.child("Pivot"); pivot_node; pivot_node = pivot_node.next_sibling())
			{			
				data_.pivot_point = osg::Vec3(
					pivot_node.attribute("x").as_double(0.0),
					pivot_node.attribute("y").as_double(0.0),
					pivot_node.attribute("z").as_double(0.0)
					);
			}

			data_.axis_up    = (au_val=="X")?xml_model_t::X_UP:
							   (au_val=="Y"?xml_model_t::Y_UP:
							   (au_val=="Z"?xml_model_t::Z_UP:
							   (au_val=="-X"?xml_model_t::NEG_X_UP:
							   (au_val=="-Y"?xml_model_t::NEG_Y_UP:
							   (au_val=="-Z"?xml_model_t::NEG_Z_UP:
							   xml_model_t::Z_UP)))));

			for (pugi::xml_node anim = root.child("Animation"); anim; anim = anim.next_sibling())
            {
                for (pugi::xml_node file = anim.first_child(); file; file = file.next_sibling())
                {
                    data_.anims[anim.attribute("name").as_string()] = file.attribute("path").as_string();
                }
            }	

            for (pugi::xml_node morph = root.child("Morph"); morph; morph = morph.next_sibling())
            {
                for (pugi::xml_node params = morph.first_child(); params; params = params.next_sibling())
                {
                    morph_params& mp = data_.morphs[morph.attribute("name").as_string()] ;
                    mp.parent = params.attribute("parent").as_string();
                    mp.source = params.attribute("source").as_string();
                    mp.target = params.attribute("target").as_string();
                }
            }

			
			
        }
        else
            std::cerr << "File not found: " << full_path;

        return data_;
    }

}
