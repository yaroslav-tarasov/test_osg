#pragma once 

#include "visitors/materials_visitor.h"
#include "utils/cpp_utils/singleton.h"

#if 0

<?xml version="1.0"?>
<root>
	<MainModel path="crow.dae" axis_up="Y"/>
	<Animation name="flap">
	<File path="flap.fbx" />
	</Animation>
	<Animation name="flap">
		<File path="flap.fbx" />
	</Animation>
	<Animation name="soar">
		<File path="soar.fbx"/>
	</Animation>
</root>

#endif



namespace avCore
{
	struct morph_params
    {
        std::string  parent;
        std::string  source;
        std::string  target;
    };

    typedef struct xml_model
	{
		typedef std::map<std::string, std::string> animations_t; 
        typedef std::map<std::string, morph_params>    morphs_t;
		std::string    main_model;
		animations_t   anims;
        morphs_t       morphs;
	} xml_model_t;

    struct ModelReader
    {
        xml_model_t  Load (std::string full_path);
    };

}



