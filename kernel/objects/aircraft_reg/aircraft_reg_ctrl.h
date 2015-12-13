#pragma once

#include "aircraft_reg_view.h"

namespace aircraft_reg
{

struct ctrl
    : view   
    , control
{
    static object_info_ptr create(kernel::object_create_t const& oc, dict_copt dict);
    ctrl(kernel::object_create_t const& oc, dict_copt dict);    

private:
    void on_object_created(object_info_ptr object) override;
    void on_object_destroying(object_info_ptr object) override;

    // control
private:
    virtual void inject_msg(net_layer::msg::run               const& msg);  
    virtual void inject_msg(net_layer::msg::container_msg     const& msg);
    virtual void inject_msg(net_layer::msg::attach_tow_msg_t  const& msg);  
    virtual void inject_msg(net_layer::msg::malfunction_msg   const& msg); 

    virtual void inject_msg(const void* data, size_t size);
	virtual void create_object(net_layer::msg::create const& msg);
    // info
private:

    // base_presentation
protected:
    void pre_update (double time) override;

protected:
    typedef size_t  extern_id_t;

    std::unordered_map<extern_id_t, object_id_t>                        e2o_;
    std::deque<net_layer::msg::run>                                  buffer_;
    std::deque<bytes_t>                                            messages_;

    // net_layer::msg::container_msg::msgs_t                          messages_;

private:
    network::msg_dispatcher<uint32_t>                                  disp_;

};

} // end of aircraft_reg