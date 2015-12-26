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
			
			data_.main_model = MainModel.attribute("path").as_string();
            
			for (pugi::xml_node anim = root.child("Animation"); anim; anim = anim.next_sibling())
            {
                for (pugi::xml_node file = anim.first_child(); file; file = file.next_sibling())
                {
                    data_.anims[anim.attribute("name").as_string()] = file.attribute("path").as_string();
                }
            }	

        }
        else
            std::cerr << "File not found: " << full_path;

        return data_;
    }

}
