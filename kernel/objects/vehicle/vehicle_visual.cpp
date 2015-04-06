#include "stdafx.h"
#include "precompiled_objects.h"

#include "vehicle_visual.h"
//#include "nodes_manager/nodes_manager_visual.h"

namespace vehicle
{

object_info_ptr visual::create(object_create_t const& oc, dict_copt dict)
{
    return object_info_ptr(new visual(oc, dict));
}

AUTO_REG_NAME(vehicle_visual, visual::create);

visual::visual(object_create_t const& oc, dict_copt dict)
    : view(oc, dict)
{
}

void visual::update(double time)
{
    view::update(time);
    
    if (nodes_management::vis_node_info_ptr(root_)->is_visible() && aerotow_)
    {
        if (!tow_visual_object_)
        {
            tow_visual_object_ = dynamic_cast<visual_system*>(sys_)->create_visual_object("towbar"/*"tube"*/) ;
            radius_ = (*tow_visual_object_)->root()->getBound().radius(); 
        }

        aircraft::info_ptr towair;

        if (tow_visual_object_ && *tow_visual_object_)
        {
            geo_base_3 base = dynamic_cast<visual_system_props*>(sys_)->vis_props().base_point;
            
            nodes_management::node_info_ptr aerotow_root = aerotow_->root();
            
            quaternion atr_quat = aerotow_root->get_global_orien();

            point_3f offset = base(tow_point_node_->get_global_pos());
            geo_point_3 air_tow_pos = geo_base_3(aerotow_root->get_global_pos())(aerotow_root->get_global_orien().rotate_vector(point_3(aerotow_->tow_point_transform().translation())));
            point_3f offset2 = base(air_tow_pos);

            cg::polar_point_3f dir(offset2 - offset);
            cpr orien(dir.course, dir.pitch, 0);
            
            

            // cg::transform_4f tr(cg::as_translation(offset), rotation_3f(orien), cg::as_scale(point_3f(1., dir.range, 1.)));
            // (*tow_visual_object_)->node()->as_transform()->set_transform(tr);

            osg::Matrix trMatrix;            
            trMatrix.setTrans(to_osg_vector3(offset));
            trMatrix.setRotate(osg::Quat(osg::inDegrees(-dir.course), osg::Z_AXIS));
            osg::Matrix scaleMatrix;
            scaleMatrix.makeScale(/*1.*/dir.range / radius_ *.25, dir.range / radius_ *.5, /*1.*/dir.range / radius_ *.25);

            (*tow_visual_object_)->root()->asTransform()->asMatrixTransform()->setMatrix(scaleMatrix);
            (*tow_visual_object_)->node()->asTransform()->asMatrixTransform()->setMatrix(trMatrix);
            (*tow_visual_object_)->set_visible(true);
        }
    }
    else
    {
        if (tow_visual_object_ && *tow_visual_object_)
            (*tow_visual_object_)->set_visible(false);
    }
}


} // vehicle
