#include "stdafx.h"
#include "precompiled_objects.h"
#include "airport_vis.h"

#include "objects/nodes_management.h"


namespace airport 
{

object_info_ptr vis::create(object_create_t const& oc, dict_copt dict)
{
    return object_info_ptr(new vis(oc, dict));
}

AUTO_REG_NAME(airport_visual, vis::create);

vis::vis(object_create_t const& oc, dict_copt dict)
    : view(oc, dict)
    , vis_sys_(dynamic_cast<visual_system *>(oc.sys))
{
#if 0
    place_lights();
    place_marking();
#endif

    on_new_settings();
}

bool vis::is_visible() const
{
    using namespace nodes_management;

    manager_ptr manager = find_first_child<manager_ptr>(this);
    Assert(manager);

    return vis_node_control_ptr(manager->get_node(0))->is_visible() ;
}

void vis::on_new_settings()
{      

    //place_lights();
    //place_marking();

    retreive_camera();
    
#if 0
    const static std::string lights_list[] = 
    {
        "apch"      , // Approach lights	
        "apsr"      , // Approach sider	
        "flash"     , // Flash
        "thsh"      , // Threshold
        "papi"      , // Papi
        "tdz"       , // Touch down zone
        "rwye"      , // Runway edge
        "rwcl"       , // Runway center-line
        "stop"      , // Taxiway stop
        "txcl"      , // Taxiway centerline
        "txe"           // Taxiway edge
    };

    for( auto it = lamps_.begin();it!=lamps_.end();++it)
    {
        auto obj = static_cast<victory::navaid_group_node *>((*it).get()); 
        for(int i=0; i<sizeof(lights_list)/sizeof(string);++i)
        {
            vector<string> tokens;
            string light_name(obj->get_name());
            
            boost::split(tokens, light_name, boost::is_any_of(":")); 
            size_t token_num = tokens.size(); 

            const int coeff = 1.7;
            lighting::navi_lights_control_t& snlc = settings_.navi_lights_control; 
            typedef  lighting::navi_lights_control_t::type_t  type_t;
            typedef  lighting::navi_lights_control_t::state_t state_t;

            try
            {
                
                if(boost::starts_with(light_name,"rwy"))
                {                
                   //  Например 
                   // #rwy01:thsh:07:R
                   // #rwy01:apch:022:L
                   // #rwy01:rwye
                   // #rwy02:rwye
                   
                   
                   int rwnum  = lexical_cast<int> (tokens[0].substr(3)) - 1;
                   if(tokens[1].find(lights_list[i])!=std::string::npos)
                   {         
                       
                       uint32_t val = 0;
                       if(token_num==2) // Для безкурсовых
                       {   
                           if (snlc.rwys[rwnum].rw_l.find(type_t(i)) != snlc.rwys[rwnum].rw_l.end())
                           {
                               val = snlc.rwys[rwnum].rw_l[type_t(i)].value;
                               obj->SetState(val>0);
                               obj->SetIntensity(val*coeff);
                           }
                       } 
                       else if(token_num>2)
                       {
                           // Проверка курса наше слабое место, много думать быстро решать
                           if(snlc.rwys[rwnum].course == lexical_cast<int>(tokens[2]) ) 
                           { 
                             if (snlc.rwys[rwnum].approach_1.find(type_t(i)) != snlc.rwys[rwnum].approach_1.end())
                             {
                                 val = settings_.navi_lights_control.rwys[rwnum].approach_1[type_t(i)].value;
                                 obj->SetState(val>0);
                                 obj->SetIntensity(val*coeff);
                             }
                           }
                           else  if(snlc.rwys[rwnum].course!=0)
                            if (snlc.rwys[rwnum].approach_2.find(type_t(i)) != snlc.rwys[rwnum].approach_2.end())
                            {
                                val = snlc.rwys[rwnum].approach_2[type_t(i)].value;
                                obj->SetState(val>0);
                                obj->SetIntensity(val*coeff);
                            }

                       }
                   
                   } 

                }
                else  if(boost::starts_with(light_name,"txe"))
                {
                    if(token_num==2)
                    {
                        auto it_s = snlc.tx_lights.find(tokens[1]);
                        if(it_s!= snlc.tx_lights.end() )
                        {
                            obj->SetState(it_s->second.edge>0);
                            obj->SetIntensity(coeff*(it_s->second.edge>0)*settings_.navi_lights_control.tx_power.edge/* *snlc.rwys[0].rw_l[lt_txe]*/); // Не ну интенсивность можно и отдельно положить
                            // а то ведь бред, да?
                        }
                    }
                    else
                    {
                        LogWarn("Not correct tag for airport " << name() << " lighting. tag:" << light_name);
                    }
                } 
                else
                if(light_name.find(lights_list[i])!=std::string::npos)
                {            
                    auto val = settings_.navi_lights_control.control[static_cast<lighting::light_type_t>(i)].value;
                    obj->SetState(val>0);
                    obj->SetIntensity(val*coeff);
                }   

            }
            catch (...)
            {
                  LogDebug ("Incorrect token in ini file = " << light_name <<  "  " << tokens[0].substr(3));
            }

        }
    }
#endif
    
    
}

void vis::on_model_changed()
{
#if 0
    place_lights();
    place_marking();
#endif
}

void vis::update(double time)
{
    view::update(time);
    
    app::main_window_ptr  mw  = vis_sys_->scene()->GetMainGUIWindow();

}

void vis::retreive_camera()
{
    using namespace nodes_management;
    
    manager_ptr manager = find_first_child<manager_ptr>(this);
    Assert(manager);

#if 0
    node_info_ptr camera = manager->find_node("camera_tower");
    camera_pos_ = camera->position().global();
#endif

    auto const& vis_nodes = vis_node_control_ptr(manager->get_node(0))->vis_nodes();
    Assert(vis_nodes.size() == 1);

    if(!vis_nodes.empty())
    {
        FindNodeVisitor findCameras("camera ",FindNodeVisitor::not_exact); 
        vis_nodes[0]->accept(findCameras);

        auto const& nl =  findCameras.getNodeList();

        if(nl.size()>0)
        {
            cg::quaternion orien =  from_osg_quat(nl[0]->asTransform()->asMatrixTransform()->getMatrix().getRotate());
            cg::point_3f pos = from_osg_vector3(nl[0]->asTransform()->asMatrixTransform()->getMatrix().getTrans());

            decart_position dpos(pos,orien);
            camera_pos_ = geo_position(dpos, ::get_base());


            app::main_window_ptr  mw  = vis_sys_->scene()->GetMainGUIWindow();
            
        }

    }


}

#if 0
void vis::place_lights()
{
    using namespace nodes_management;
    using namespace victory;

    manager_ptr manager = find_first_child<manager_ptr>(this);
    Assert(manager);

    auto const& vict_nodes = vis_node_control_ptr(manager->get_node(0))->vis_nodes();
    Assert(vict_nodes.size() == 1);

    victory::ISceneGraphPtr scene_graph = vis_sys_->victory()->scenegraph();

    if(!vict_nodes.empty())
    {

    victory::group_node_ptr parent      = vict_nodes.front()->as_group();

    fill_navids(
        fs::path(cfg().path.data) / "lights" / (get_model(settings_.icao_code) + ".txt"), 
        scene_graph, 
        lamps_, 
        parent, 
        lights_offset(settings_.icao_code) + point_3f(settings_.lights_offset));   

   }
}
#endif

#if 0
void vis::place_marking()
{
    geo_base_2 base_geo_pos = dynamic_cast<visual_system_props*>(vis_sys_)->vis_props().base_point;

    if (cg::distance2d(base_geo_pos, pos()) > 15000.)
        return ;

    auto decalrender_ptr = vis_sys_->scene()->get_scene_decal_renderer();
    decalrender_ptr->clear();

    struct kind_info
    {
        bool show;
        float width;
        cg::colorf col;
    };

    static const kind_info ways_kind_info[atc::airport::AS_SERVICE + 1] = {
        { true, 0.40f, cg::colorf(0.78f, 0.77f, 0.16f) },
        { true, 1.00f, cg::colorf(0.80f, 0.80f, 0.80f) },
        { true, 0.25f, cg::colorf(0.70f, 0.70f, 0.70f) }
    };

    static std::vector<cg::point_2f> way_pts;

    for (auto it = data_.ways.rbegin(), it_end = data_.ways.rend(); it != it_end; ++it)
    {
        const auto & way = *it;

        if (way.kind < 0 || !ways_kind_info[way.kind].show)
            continue;
        const auto & way_kind = ways_kind_info[way.kind];

        way_pts.resize(0);
        std::transform(way.points.begin(), way.points.end(), std::back_inserter(way_pts), base_geo_pos);

        decalrender_ptr->add_polyline(way_pts, way_kind.col, way_kind.width);
    }
}
#endif

geo_point_3 vis::camera_pos() const
{
    return camera_pos_.pos;
}

cpr vis::camera_orien() const
{
    return camera_pos_.orien.cpr();
}

geo_point_3 vis::pos() const
{
    return camera_pos_.pos;
}

cpr vis::orien() const
{
    return camera_pos_.orien.cpr();
}



double vis::zoom() const
{
    return 1.;
}


} // airport 
