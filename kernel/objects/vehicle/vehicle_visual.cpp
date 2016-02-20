#include "stdafx.h"
#include "precompiled_objects.h"

#include "vehicle_visual.h"
#include "common/text_label.h"


namespace vehicle
{
	const double visual::fs_end_duration_  = 10;

    struct visual::tow_support
    {
        tow_support(visual_object_ptr obj,const std::string& tow_type )
           : tow_visual_object_(obj)
           , body_s_(nullptr) 
        {
            if(tow_type=="towbar")
            {   
                init_towbar();
                update_f_ = std::bind(&tow_support::towbar_update,this,sp::_1,sp::_2);
            }
            else
                update_f_ = std::bind(&tow_support::tube_update,this,sp::_1,sp::_2);
        }

        inline void update( cg::polar_point_3f const  &dir, point_3f offset )
        {
                update_f_(dir,offset);
        }

    private:

        inline void init_towbar()
        {
            const auto root_node = (tow_visual_object_)->root();
			body_s_ = findFirstNode(root_node,"body_s");
            body_b_ = findFirstNode(root_node,"body_b");
            body_a_ = findFirstNode(root_node,"body_a");

            osg::ComputeBoundsVisitor cbvs;
            body_s_->accept( cbvs );
            const osg::BoundingBox bb_s = cbvs.getBoundingBox();

            osg::ComputeBoundsVisitor cbvb;
            body_b_->accept( cbvb );
            const osg::BoundingBox bb_b = cbvb.getBoundingBox();

            osg::ComputeBoundsVisitor cbva;
            body_a_->accept( cbva );
            const osg::BoundingBox bb_a = cbva.getBoundingBox();

            osg::ComputeBoundsVisitor cbv;
            root_node->accept( cbv );
            const osg::BoundingBox bb_ = cbv.getBoundingBox();

            radius_   = abs(bb_.yMax() - bb_.yMin())/2.0;//(*tow_visual_object_)->root()->getBound().radius();

            radius_s_ = abs(bb_s.yMax() - bb_s.yMin())/2.0;//body_s_->getBound().radius();
            radius_a_ = abs(bb_a.yMax() - bb_a.yMin())/2.0;//body_a_->getBound().radius(); 
            radius_b_ = abs(bb_b.yMax() - bb_b.yMin())/2.0;//body_b_->getBound().radius();
        }

        void towbar_update( cg::polar_point_3f const  &dir, point_3f offset )
        {
            const double scale_coeff = (dir.range -  (radius_a_ + radius_b_)*2) / radius_s_ *.5; 

            osg::Matrix trMatrix;            
            trMatrix.setTrans(to_osg_vector3(offset));
            trMatrix.setRotate(osg::Quat(osg::inDegrees(-dir.course), osg::Z_AXIS));
            osg::Matrix scaleMatrix;
            FIXME("All good, but 1.1")
            scaleMatrix.makeScale(1., scale_coeff * 1.1 , 1.);


            // (*tow_visual_object_)->root()->asTransform()->asMatrixTransform()->setMatrix(scaleMatrix);
            {
                const double dy = (scale_coeff - 1) * radius_s_;
                body_b_->asTransform()->asMatrixTransform()->setMatrix(osg::Matrix::translate(osg::Vec3(0,-(0/*dir.range *.5 - radius_*/),0)) );
                body_a_->asTransform()->asMatrixTransform()->setMatrix(osg::Matrix::translate(osg::Vec3(0,2*dy/*dir.range *.5 - radius_*/,0)));
                osg::Matrix m3;
                m3.setTrans(osg::Vec3(0,-(0.05*dy/*dir.range *.5 - radius_*/),0));
                scaleMatrix.postMult(m3);
                body_s_->asTransform()->asMatrixTransform()->setMatrix(scaleMatrix);
            }

            (tow_visual_object_)->node()->asTransform()->asMatrixTransform()->setMatrix(trMatrix);
        }

        void tube_update(cg::polar_point_3f const  &dir, point_3f offset )
        {
            osg::Matrix trMatrix;
            trMatrix.setTrans(to_osg_vector3(offset));
            trMatrix.setRotate(osg::Quat(osg::inDegrees(-dir.course), osg::Z_AXIS));
            osg::Matrix scaleMatrix;
            scaleMatrix.makeScale(1., dir.range, 1.);

            (tow_visual_object_)->root()->asTransform()->asMatrixTransform()->setMatrix(scaleMatrix);
            (tow_visual_object_)->node()->asTransform()->asMatrixTransform()->setMatrix(trMatrix);
        }

        double                radius_;
        double                radius_s_;
        double                radius_a_;
        double                radius_b_;
        osg::Node *           body_s_;
        osg::Node *           body_a_;
        osg::Node *           body_b_;

        visual_object_ptr     tow_visual_object_;
        typedef std::function<void(cg::polar_point_3f const &dir, point_3f offset )>  update_t;
        update_t              update_f_; 
    };
}


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


#ifndef ASYNC_OBJECT_LOADING 
    visual_system* vsys = dynamic_cast<visual_system*>(sys_);
    label_object_ = vsys->create_visual_object(nm::node_control_ptr(root_),"text_label.scg");
    ls_ = boost::make_shared<visual_objects::label_support>(label_object_, settings_.custom_label);
#endif
}

void visual::settings_changed()
{
    view::settings_changed();
    ls_->set_text(settings_.custom_label);
}

void visual::update(double time)
{
    view::update(time);
    
    if (nodes_management::vis_node_info_ptr(root_)->is_visible() )
    {

#ifdef ASYNC_OBJECT_LOADING 
		if(!label_object_ && nm::vis_node_control_ptr(root_)->vis_nodes().size()>0)
		{
			visual_system* vsys = dynamic_cast<visual_system*>(sys_);
			label_object_ = vsys->create_visual_object(nm::node_control_ptr(root_),"text_label.scg", 0 ,false);
		    ls_ = boost::make_shared<visual_objects::label_support>(label_object_, settings_.custom_label);
		}
#endif
		
		if( aerotow_)
		{
			if (!tow_visual_object_)
			{

				const std::string tow_type = "towbar"; // "towbar"  "tube"
				tow_visual_object_ = dynamic_cast<visual_system*>(sys_)->create_visual_object(tow_type, 0 ,  false) ;
				ts_ = boost::make_shared<tow_support>(*tow_visual_object_, tow_type);
			}


			aircraft::info_ptr towair;

			if (tow_visual_object_ && *tow_visual_object_)
			{
				geo_base_3 base = dynamic_cast<visual_system_props*>(sys_)->vis_props().base_point;
            
				nodes_management::node_info_ptr aerotow_root = aerotow_->root();
            
				quaternion atr_quat = aerotow_root->get_global_orien();

				point_3f offset = base(/*tow_point_node_*/current_tow_point_node_->get_global_pos());
				geo_point_3 air_tow_pos = geo_base_3(aerotow_root->get_global_pos())(aerotow_root->get_global_orien().rotate_vector(point_3(aerotow_->tow_point_transform().translation())));
				point_3f offset2 = base(air_tow_pos);

				cg::polar_point_3f dir(offset2 - offset);
				cpr orien(dir.course, dir.pitch, 0);

				// cg::transform_4f tr(cg::as_translation(offset), rotation_3f(orien), cg::as_scale(point_3f(1., dir.range, 1.)));
				// (*tow_visual_object_)->node()->as_transform()->set_transform(tr);
            
				ts_->update(dir, offset);

				(*tow_visual_object_)->set_visible(true);
			}
		}
        else
        {
            if(ts_)
                ts_.reset();

            if (tow_visual_object_ && *tow_visual_object_)
                (*tow_visual_object_)->set_visible(false);
        }
		
		if ( burning_plane_ )
		{
			last_fs_time_ = time;
		}

		if ( turret_node_ && burning_plane_)
		{
			if(!foam_stream_object_)
			{
				if (true)
				{
				   foam_stream_object_ = dynamic_cast<visual_system*>(sys_)->create_visual_object(turret_node_,"sfx//foam_stream.scg") ;
				   foam_stream_sfx_weak_ptr_ = nullptr;
				   if (auto foam_stream_node = findFirstNode(foam_stream_object_->node().get(),"FoamStreamFx"))
				   {
					   foam_stream_sfx_weak_ptr_ = dynamic_cast<FoamStreamSfxNode *>(foam_stream_node);
				   }

                   fs_start_time_ = time;
				}
			}
			
			if(foam_stream_object_)
			{
				geo_base_3 base = dynamic_cast<visual_system_props*>(sys_)->vis_props().base_point;
				geo_base_3 root_pos = root_->get_global_pos();
				quaternion root_orien = root_->get_global_orien();

				if (nodes_management::vis_node_info_ptr(root_)->is_visible())
				{
#if 0
					geo_base_3 node_pos = turret_node_->get_global_pos();
					quaternion node_orien = turret_node_->get_global_orien();
					point_3f pos = base(node_pos);
#endif
                    nodes_management::node_info_ptr burning_plane_root = burning_plane_->root();
					cg::geo_point_3 bp_geo_root = burning_plane_root->get_global_pos();
					
					geo_base_3 node_gpos = turret_node_->get_global_pos();
					double dist = cg::norm(geo_base_3(root_pos)(bp_geo_root)); 
					// double speed = cg::sqrt(dist*9.8/cg::abs(sin(2.0*cg::grad2rad()*15.0)) );
                    const double dt = (time - *fs_start_time_)/4.0;
                    double speed = cg::sqrt(dist*9.8/cg::abs(sin(cg::grad2rad()*15.0)) * 0.5 ) * ((dt)<1.0?dt:1.0);
                    
                    if (dist > 50.0)
                    {
                      if(!fs_stop_time_)
                          fs_stop_time_ = time;
                      
                      const double dt = (time - *fs_stop_time_)/4.0;
                      speed = cg::sqrt(dist*9.8/cg::abs(sin(cg::grad2rad()*15.0)) * 0.5 ) * ((dt)<1.0?(1.0-dt):0.0);
                    }

					// foam_stream_object_->node()->as_transform()->set_transform(cg::transform_4f(cg::as_translation(pos), cg::rotation_3f(node_orien.rotation())));
					if(fs_stop_time_ && cg::eq(speed, 0.0) )
                    {
                        foam_stream_object_->set_visible(false);
                        burning_plane_.reset();
                    }
                    else
                        foam_stream_object_->set_visible(true);

					if (foam_stream_sfx_weak_ptr_)
					{
						auto const intensity = 1.0f;
						
						fs_factor_ = intensity; 
						
						foam_stream_sfx_weak_ptr_->setFactor(fs_factor_ * cg::clamp(0., fs_end_duration_, 1., 0.)(time-last_fs_time_));
						foam_stream_sfx_weak_ptr_->setIntensity(fs_factor_ * 120);
						foam_stream_sfx_weak_ptr_->setEmitterWorldSpeed(cg::point_3(0.0,speed,0.0));
					}
				}
				else
					foam_stream_object_->set_visible(false);
			}


		}
    }
    else
    {
        if (tow_visual_object_ && *tow_visual_object_)
            (*tow_visual_object_)->set_visible(false);
    }

#if 1
	if (foam_stream_object_ && time > last_fs_time_ + fs_end_duration_ + foam_stream_sfx_weak_ptr_->getMaxParticleLifetime())
	{
		foam_stream_sfx_weak_ptr_ = nullptr;
		foam_stream_object_.reset();
	}
#endif
}


} // vehicle
