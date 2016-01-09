#pragma once

#include "objects_reg_view.h"

namespace objects_reg
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

private:
    
    void on_detach_tow (uint32_t, decart_position const& );

    // control
private:
    void inject_msg   (net_layer::msg::run               const& msg);  
    void inject_msg   (net_layer::msg::container_msg     const& msg);
    void inject_msg   (net_layer::msg::attach_tow_msg_t  const& msg);  
    void inject_msg   (net_layer::msg::detach_tow_msg_t  const& msg); 
    void inject_msg   (net_layer::msg::malfunction_msg   const& msg); 
	void inject_msg   (net_layer::msg::fire_fight_msg_t  const& msg); 

    virtual void inject_msg   (const void* data, size_t size);
	virtual void create_object(net_layer::msg::create const& msg);
    virtual void set_sender   (remote_send_f s) override;

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
    
    remote_send_f                                                      send_;
private:
    network::msg_dispatcher<uint32_t>                                  disp_;

private:
    connection_holder                                           conn_holder_;
};

} // end of objects_reg