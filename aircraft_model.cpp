#include "stdafx.h"
#include "aircraft_model.h"

namespace aircraft
{

// from view
#pragma region view
cg::transform_4 const&  model::tow_point_transform() const
{
    return tow_point_transform_;
}

nodes_management::node_info_ptr model::root() const
{
    //return nodes_manager_->get_node(0);  // FIXME отступаем от исходной модели
    return nodes_manager_->find_node("root");
}

bool model::malfunction(malfunction_kind_t kind) const
{
    return malfunctions_[kind];
}

cg::geo_point_3 const& model::pos() const
{
    static cg::geo_point_3 pp;
    return pp;//fms_info_->get_state().dyn_state.pos;
}

cg::point_3 model::dpos() const
{
    return cg::point_3();// cg::polar_point_3(fms_info_->get_state().dyn_state.TAS, fms_info_->get_state().orien().course, fms_info_->get_state().orien().pitch);
}

cg::cpr model::orien() const
{
    return cg::cpr();//fms_info_->get_state().orien();
}

#pragma  endregion


}