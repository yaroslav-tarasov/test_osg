#include "stdafx.h"
#include "precompiled_objects.h"

#include "vehicle_visual.h"
#include "nodes_manager/nodes_manager_visual.h"

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
            tow_visual_object_ = dynamic_cast<visual_system*>(sys_)->create_visual_object("tube") ;
        
        aircraft::info_ptr towair;
        
        auto nm = kernel::find_first_child<nodes_management::manager_ptr>(aerotow_);
        uint32_t nm_id = kernel::object_info_ptr(nm)->object_id();

        const kernel::object_collection  *  col = dynamic_cast<kernel::object_collection *>(sys_);
        visit_objects<aircraft::info_ptr>(col, [this,&nm_id](aircraft::info_ptr air)->bool
        {    
            auto nm = kernel::find_first_child<nodes_management::manager_ptr>(air);
            if (kernel::object_info_ptr(nm)->object_id() == nm_id)
            {              
                //towair = air; 
                nodes_management::visual_manager_ptr(nm)->visual_object();
                return false;
            }

            return true;
        });

        if (tow_visual_object_ && *tow_visual_object_)
        {
            geo_base_3 base = dynamic_cast<visual_system_props*>(sys_)->vis_props().base_point;
            
            nodes_management::node_info_ptr aerotow_root = aerotow_->root();
            
            quaternion atr_quat = aerotow_root->get_global_orien();

            point_3f damned_offset = from_osg_vector3((*tow_visual_object_)->pat()->asTransform()->asPositionAttitudeTransform()->getPosition());

            point_3f offset = base(tow_point_node_->get_global_pos());
            geo_point_3 air_tow_pos = geo_base_3(aerotow_root->get_global_pos())(/*aerotow_root->get_global_orien()*/quaternion(cpr(atr_quat.get_course(),atr_quat.get_pitch(),atr_quat.get_roll())).rotate_vector(damned_offset + point_3(aerotow_->tow_point_transform().translation())));
            point_3f offset2 = base(air_tow_pos);
            //point_3f offset2 = base(aerotow_root->get_global_pos());

            cg::polar_point_3f dir(offset2 - offset);
            cpr orien(dir.course, dir.pitch, 0);
            
            

            FIXME(TBD)
            cg::transform_4f tr(cg::as_translation(offset), rotation_3f(orien), cg::as_scale(point_3f(1., dir.range, 1.)));
            //(*tow_visual_object_)->node()->as_transform()->set_transform(tr);

            std::stringstream cstr;
            cstr << "dir.course : " << dir.course  
                 << "  get_course() : " << aerotow_root->get_global_orien().get_course() 
                 << "  get_roll() : " << aerotow_root->get_global_orien().get_roll()  
                 << "  get_roll() : " << aerotow_root->get_global_orien().get_roll()                
                <<"\n" ;
 
            OutputDebugString(cstr.str().c_str());    

            osg::Matrix trMatrix;
            trMatrix.setTrans(to_osg_vector3(offset));
            trMatrix.setRotate(osg::Quat(osg::inDegrees(-dir.course), osg::Z_AXIS));
            osg::Matrix scaleMatrix;
            scaleMatrix.makeScale(1., dir.range, 1.);

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
