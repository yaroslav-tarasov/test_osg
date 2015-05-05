#include "stdafx.h"
#include "precompiled_objects.h"
#include "aircraft_visual.h"

namespace aircraft
{

	object_info_ptr visual::create(kernel::object_create_t const& oc, dict_copt dict)
	{   
		return object_info_ptr(new visual(oc, dict));
	}

	AUTO_REG_NAME(aircraft_visual, visual::create);

	visual::visual( kernel::object_create_t const& oc, dict_copt dict )
		: view(oc,dict)
	{
        visual_system* vsys = dynamic_cast<visual_system*>(sys_);
        
        nm::visit_sub_tree(get_nodes_manager()->get_node_tree_iterator(root()->node_id()), [this](nm::node_info_ptr n)->bool
        {
            if (boost::starts_with(n->name(), "engine_l"))
            {
                this->engine_node_ = n;
                return false;
            }
            return true;
        });
	}

    void visual::update(double time)
    {
        view::update(time);
        update_len(time);

        double dt = time - (last_update_ ? *last_update_ : 0);
        //if (cg::eq_zero(dt))
        //    return;

        bool has_smoke = malfunction(MF_FIRE_ON_BOARD) || malfunction(MF_SMOKE_ON_BOARD);
        
        //if (has_smoke)
        //{
        //    last_fire_time_ = time;
        //}

        geo_base_3 base = dynamic_cast<visual_system_props*>(sys_)->vis_props().base_point;
        geo_base_3 root_pos = root()->get_global_pos();
        quaternion root_orien = root()->get_global_orien();

        
        if (smoke_object_ && engine_node_)
        {
            if (nodes_management::vis_node_info_ptr(root())->is_visible())
            {
                geo_base_3 node_pos   = engine_node_->get_global_pos();
                quaternion node_orien = engine_node_->get_global_orien();
                point_3f   pos        = base(node_pos);

                quaternion sr = from_osg_quat(smoke_object_->node()->asTransform()->asMatrixTransform()->getMatrix().getRotate());
                const float angular_velocity = 3 * 2 * osg::PI/60.0;
                quaternion des_orien;
                des_orien = sr * quaternion(cpr(cg::rad2grad() * angular_velocity * dt,0,0));
                quaternion omega_rel     = cg::get_rotate_quaternion(node_orien,des_orien)/*.rot_axis().omega()*/ / (dt);                
                
                // smoke_object_->node()->as_transform()->set_transform(cg::transform_4f(cg::as_translation(pos), cg::rotation_3f(node_orien.rotation())));
                // smoke_object_->node()->asTransform()->asMatrixTransform()->setMatrix(to_osg_transform(cg::transform_4f(cg::as_translation(pos), cg::rotation_3f(des_orien.rotation()/*node_orien.rotation()*/))));
                
                smoke_object_->set_visible(true);

            }
            else
                smoke_object_->set_visible(false);
        }

        last_update_ = time;
    }

    void visual::on_malfunction_changed( malfunction_kind_t kind )
    {
        if (kind == MF_FIRE_ON_BOARD || kind == MF_SMOKE_ON_BOARD)
        {
            visual_system* vsys = dynamic_cast<visual_system*>(sys_);

            bool has_smoke = malfunction(MF_FIRE_ON_BOARD) || malfunction(MF_SMOKE_ON_BOARD);

            if (!smoke_object_ && has_smoke && engine_node_)
            {
                smoke_object_ = vsys->create_visual_object(nm::node_control_ptr(engine_node_),"sfx//smoke.scg");
            }


        }
    }

}


