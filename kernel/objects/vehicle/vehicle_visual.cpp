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
    , body_s_(nullptr) 
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
            body_s_ = findFirstNode((*tow_visual_object_)->root(),"body_s");
            body_b_ = findFirstNode((*tow_visual_object_)->root(),"body_b");
            body_a_ = findFirstNode((*tow_visual_object_)->root(),"body_a");
            
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
            (*tow_visual_object_)->root()->accept( cbv );
            const osg::BoundingBox bb_ = cbv.getBoundingBox();

            radius_   = abs(bb_.yMax() - bb_.yMin())/2.0;//(*tow_visual_object_)->root()->getBound().radius();

            radius_s_ = abs(bb_s.yMax() - bb_s.yMin())/2.0;//body_s_->getBound().radius();
            radius_a_ = abs(bb_a.yMax() - bb_a.yMin())/2.0;//body_a_->getBound().radius(); 
            radius_b_ = abs(bb_b.yMax() - bb_b.yMin())/2.0;//body_b_->getBound().radius();

            logger::need_to_log(true);

            LOG_ODS_MSG ("radius_s_:  "  << radius_s_  << " : " << bb_s.yMax() << " : "<< bb_s.yMin()
                << "    radius_a_: " << radius_a_   << " : " << bb_a.yMax() << " : "<< bb_a.yMin()
                << "    radius_b_: " << radius_b_  << " : " << bb_b.yMax() << " : "<< bb_b.yMin() 
                << "    radius_: " << radius_ << " : " << bb_.yMax() << " : "<< bb_.yMin()
                << "\n");

            logger::need_to_log(false);

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
            
            const double scale_coeff = (dir.range -  (radius_a_ + radius_b_)*2) / radius_s_ *.5; 
            
            osg::Matrix trMatrix;            
            trMatrix.setTrans(to_osg_vector3(offset));
            trMatrix.setRotate(osg::Quat(osg::inDegrees(-dir.course), osg::Z_AXIS));
            osg::Matrix scaleMatrix;
            scaleMatrix.makeScale(1., scale_coeff * 1.1 , 1.);
            
             
            // (*tow_visual_object_)->root()->asTransform()->asMatrixTransform()->setMatrix(scaleMatrix);
            {
                const double dy = (scale_coeff - 1) * radius_s_;
                osg::Matrix trMatrix;            
                trMatrix.setTrans(osg::Vec3(0,-(0/*dir.range *.5 - radius_*/),0));
                body_b_->asTransform()->asMatrixTransform()->setMatrix(trMatrix);
                trMatrix.setTrans(osg::Vec3(0,2*dy/*dir.range *.5 - radius_*/,0));
                body_a_->asTransform()->asMatrixTransform()->setMatrix(trMatrix);
                osg::Matrix m3;
                m3.setTrans(osg::Vec3(0,-(0.05*dy/*dir.range *.5 - radius_*/),0));
                scaleMatrix.postMult(m3);
                body_s_->asTransform()->asMatrixTransform()->setMatrix(scaleMatrix);
            }

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
