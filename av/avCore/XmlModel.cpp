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

    bool  ModelReader::Load (const std::string& full_path, xml_model_t& data  )
    {
        pugi::xml_document  doc;

        bool l = doc.load_file(full_path.c_str());
        if(l)
        {
            pugi::xml_node root = doc.child("root");
			
			pugi::xml_node MainModel =  root.child("MainModel");

			
            pugi::xml_node file_node = MainModel.child("File");
            if(file_node)
               data.main_model  = file_node.attribute("path").as_string();
            else
               data.main_model   = MainModel.attribute("path").as_string();
            
            std::string au_val;
            data.lod3 = true;
			data.hw_instanced = false;

			
            pugi::xml_node param_node = MainModel.child("Parameters");
            if(param_node)
            {
                data.scale        = param_node.attribute("scale").as_float(1.0);
                au_val			  = param_node.attribute("axis_up").as_string("Z");
                data.lod3		  = param_node.attribute("lod3").as_string("on")!=std::string("off");
				data.hw_instanced = param_node.attribute("hw_instanced").as_string("no")==std::string("yes");
				data.hw_data      = param_node.attribute("hw_data").as_string("");
            }
            else
            {
                data.scale = MainModel.attribute("scale").as_float(1.0);
                au_val      = MainModel.attribute("axis_up").as_string("Z");
            }


			for (pugi::xml_node pivot_node = MainModel.child("Pivot"); pivot_node; pivot_node = pivot_node.next_sibling())
			{			
				data.pivot_point = osg::Vec3(
					pivot_node.attribute("x").as_double(0.0),
					pivot_node.attribute("y").as_double(0.0),
					pivot_node.attribute("z").as_double(0.0)
					);
			}

			data.axis_up    = (au_val=="X")?xml_model_t::X_UP:
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
                    data.anims[anim.attribute("name").as_string()] = file.attribute("path").as_string();
                }
            }	

            for (pugi::xml_node morph = root.child("Morph"); morph; morph = morph.next_sibling())
            {
                for (pugi::xml_node params = morph.first_child(); params; params = params.next_sibling())
                {
                    morph_params& mp = data.morphs[morph.attribute("name").as_string()] ;
                    mp.parent = params.attribute("parent").as_string();
                    mp.source = params.attribute("source").as_string();
                    mp.target = params.attribute("target").as_string();
                }
            }
 			
        }
        else
        {
            std::cerr << "ModelReader: File not found: " << full_path;
            return false;
        }

        return true;
    }

    bool  ModelReader::Load (const std::string& full_path, xml_scene_t& data  )
    {
        pugi::xml_document  doc;

        bool l = doc.load_file(full_path.c_str());
        if(l)
        {
            pugi::xml_node root = doc.child("root");

            pugi::xml_node MainModel =  root.child("MainModel");


            pugi::xml_node file_node = MainModel.child("File");
            if(file_node)
                data.main_model  = file_node.attribute("path").as_string();
            else
                data.main_model  = MainModel.attribute("path").as_string();

            std::string au_val;

            pugi::xml_node param_node = MainModel.child("Parameters");
            if(param_node)
            {
                data.scale        = param_node.attribute("scale").as_float(1.0);
                au_val			  = param_node.attribute("axis_up").as_string("Z");
            }
            else
            {
                data.scale = MainModel.attribute("scale").as_float(1.0);
                au_val      = MainModel.attribute("axis_up").as_string("Z");
            }


            for (pugi::xml_node pivot_node = MainModel.child("Pivot"); pivot_node; pivot_node = pivot_node.next_sibling())
            {			
                data.pivot_point = osg::Vec3(
                    pivot_node.attribute("x").as_double(0.0),
                    pivot_node.attribute("y").as_double(0.0),
                    pivot_node.attribute("z").as_double(0.0)
                    );
            }

            data.axis_up    = (au_val=="X")?xml_model_t::X_UP:
                (au_val=="Y"?xml_model_t::Y_UP:
                (au_val=="Z"?xml_model_t::Z_UP:
                (au_val=="-X"?xml_model_t::NEG_X_UP:
                (au_val=="-Y"?xml_model_t::NEG_Y_UP:
                (au_val=="-Z"?xml_model_t::NEG_Z_UP:
                xml_model_t::Z_UP)))));
            
            for (pugi::xml_node cam = root.child("Camera"); cam; cam = cam.next_sibling())
            {
                camera_params cp;
                cp.pos = osg::Vec3(
                    cam.attribute("x").as_double(0.0),
                    cam.attribute("y").as_double(0.0),
                    cam.attribute("z").as_double(0.0)
                    );

                cp.course =  cam.attribute("course").as_float(0.f);
                data.cams.push_back(cp);
            }	

			for (pugi::xml_node mn = root.child("MaskNode"); mn; mn = mn.next_sibling())
			{

				std::string name = 	mn.attribute("name").as_string("");
				if (name.size()>0)
					data.mask_nodes.push_back(name);
			}	

			for (pugi::xml_node obj = root.child("Object"); obj; obj = obj.next_sibling())
			{

				const std::string main_model = 	obj.attribute("model").as_string("");
				const std::string data_file = 	obj.attribute("data_file").as_string("");
				if (main_model.size()>0 && data_file.size()>0)
				{
					xml_static_model v;
					v.main_model = main_model;
					v.data_file  = data_file;
					data.objs.push_back(v);
				}
			}	

        }
        else
        {
            std::cerr << "ModelReader: File not found: " << full_path;
            return false;
        }

        return true;
    }

	bool  ModelReader::Load (const std::string& full_path, xml_object_data_t& data )
	{
		pugi::xml_document  doc;

		bool l = doc.load_file(full_path.c_str());
		if(l)
		{
			pugi::xml_node root = doc.child("root");

			for (pugi::xml_node obj = root.child("value"); obj; obj = obj.next_sibling())
			{

				const float  x = 	obj.attribute("x").as_float(0.f);
                const float  y = 	obj.attribute("y").as_float(0.f);
				const float  z = 	obj.attribute("z").as_float(0.f);
				const float  h = 	obj.attribute("h").as_float(0.f);

				xml_object_data v;
				v.pos =  osg::Vec4(x,y,z,h);
				data.push_back(v);
			}

		}
		else
		{
			std::cerr << "ModelReader: File not found: " << full_path;
			return false;
		}

		return true;
	}

}
