#pragma once 

#include "visitors/materials_visitor.h"
#include "utils/cpp_utils/singleton.h"

#if 0

<?xml version="1.0"?>
<root>
	<MainModel path="crow.dae" axis_up="Y" scale ="0.025"/>

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

<root>
    <MainModel>
        <File path="human.fbx" />
        <Parameters axis_up="Y" scale="0.13" lod3="off" />
        <Pivot x="0" y="8.26" z="0" />
    </MainModel>
    <Animation name="idle">
        <File path="bashful.fbx" />
    </Animation>
    <Animation name="run">
    <File path="medium_run.fbx" />
        </Animation>
    <Animation name="walk">
        <File path="female_walk.fbx"/>
    </Animation>
</root>

<MainModel path="crow.dae" >
	<Parameters axis_up="Y" scale="0.02" hw_instanced="yes" />
</MainModel>

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
		enum up_axis_t {X_UP,Y_UP,Z_UP,NEG_X_UP,NEG_Y_UP,NEG_Z_UP};
		typedef std::map<std::string, std::string> animations_t; 
        typedef std::map<std::string, morph_params>    morphs_t;
		std::string    main_model;
		animations_t   anims;
        morphs_t       morphs;
		float		   scale;
		up_axis_t      axis_up;
		osg::Vec3      pivot_point;
        bool           lod3;
		bool		   hw_instanced;
	} xml_model_t;

    struct ModelReader
    {
        xml_model_t  Load (std::string full_path);
    };

}



