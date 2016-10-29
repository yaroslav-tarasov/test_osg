#pragma once

#include "messages.h"

#include "kernel/systems/systems_base.h"
#include "kernel/systems.h"
#include "kernel/msg_proxy.h"

#include "alloc/pool_stl.h"
#include "common/randgen.h"

#include "network/msg_dispatcher.h"

#if 0
#include "python/utils.h"
#endif

namespace kernel
{

struct msg_keeper;

struct  SYSTEMS_API system_base
	: system            
	, system_session    
	, objects_factory
	, object_collection 
	, boost::enable_shared_from_this<system_base>
{
	system_base(system_kind kind, msg_service& service, std::string const &objects_file_name);
	//! деструктор
	~system_base();
	// system
protected:
	system_kind         kind                () const                    override;
	void                update              (double time)               override;
	void                on_msg              (binary::bytes_cref bytes)  override;
	void                load_exercise       (dict_cref dict)            override;
	void                save_exercise       (dict_ref  dict, bool safe_key) const      override;
	optional<double>    update_time         () const                    override;
	optional<double>    last_update_time    () const                    override;
	double              atc_update_period   () const                    override;

	// system_session
protected:
	void on_session_loaded      ();
	void on_session_stopped     ();
	void on_time_factor_changed (double time, double factor);

	// objects_factory
protected:
	object_info_ptr create_object            (object_class_ptr hierarchy_class, std::string const &name)     override;
	object_info_ptr create_object            (obj_create_data const& descr)                                  override;
	object_info_ptr load_object_hierarchy    (dict_cref dict)                                                override;
	void            save_object_hierarchy    (object_info_ptr objinfo, dict_ref dict, bool safe_key) const   override;    
	object_class_vector const& object_classes() const                                                        override;
	object_class_ptr get_object_class        (std::string const& name) const                                 override;

	std::string     generate_unique_name     (std::string const &init_name) const override;

private:
	object_info_ptr local_create_object      (obj_create_data const& data);
	void            pack_exercise            (const creating_objects_list_t& co , dict_ref dict, bool safe_key) /*const*/;

private:
	void check_destroy(std::vector<object_info_wptr> const& objs_to_destroy);

	// object_collection
protected:
	objects_t const&    root_objects    () const override;
	object_info_ptr     get_object      (obj_id_t object_id) const override;
	void                destroy_object  (obj_id_t object_id) override;

protected:
	void send_obj_message(size_t object_id, binary::bytes_cref bytes, bool sure, bool just_cmd);

protected:
	void process_destroy_object( size_t object_id );

protected:
	virtual void do_pre_update  (double time);
	virtual void do_update      (double time);
	virtual void do_post_update (double time);
	virtual void do_update_atc  (double time);

protected:
	network::msg_dispatcher<>&  msg_disp();

protected:
	typedef 
		ph_map<size_t, object_info_wptr>::map_t
		weak_objects_t;

	//! дерево объектов (физически map)
	objects_t       root_objects_; // roots
	//! линейный набор (map) ссылок на объекты
	weak_objects_t  objects_     ; // plain object list 

protected:
	//     typedef weak_ptr<msg_keeper>    msg_keeper_ptr;
	//     typedef list<msg_keeper_ptr>    msg_keeper_ptrs; 

	object_info_ptr create_object_hierarchy_impl(object_class_ptr hierarchy_class       , std::string const &name, bool is_root);
	object_info_ptr load_object_hierarchy_impl  (object_class_ptr parent_hierarchy_class, dict_cref dict, bool is_root, bool read_id);

protected:
	vector<string> const&   auto_object_order ();
	vector<obj_id_t>        remove_roots_order();

protected:
	typedef ph_set<obj_id_t>::set_t obj_set_t;

protected:
	system_kind     kind_;
	msg_service_reg msg_service_;

	optional<double> update_time_;
	optional<double> last_update_time_;
	optional<double> last_update_atc_;

protected:
	object_class_ptr root_;

private:
	// we do not want to assign equal objects id to previously created object
	// even if it has been already removed - such a strategy makes history recording easier 
	obj_set_t used_ids_;
	randgen<> id_randgen_;

private:
	void     register_obj_id   (obj_id_t id);
	obj_id_t generate_object_id();

private:
	void fire_object_created(object_info_ptr obj);

private:
	void on_object_created(msg::object_created const& msg);
	void on_object_destroy(msg::destroy_object const& msg);
	void on_object_msg    (msg::object_msg     const& msg);
	void on_container_msg (msg::container_msg  const& msg);

private:
	struct msgs_blocker;
	void block_obj_msgs  (bool block);
	bool obj_msgs_blocked() const;

private:
	DECL_LOGGER("system_base");

private:
	//! диспетчер сообщений для всех объектов "подсистем"
	network::msg_dispatcher<> msg_disp_;

private:
	msg::container_msg::msgs_t tcp_messages_;
	msg::container_msg::msgs_t udp_messages_;

	size_t       udp_messages_size_;
	const size_t udp_msg_threshold_;

private:
	obj_set_t obj_ids_to_destroy_;
	obj_set_t destroyed_objects_;

private:
	bool create_object_lock_;

private:
	size_t       block_obj_msgs_counter_;

private:
	typedef msg::object_msg::msg_protocol_t msg_protocol_t;

	optional<msg_protocol_t> msg_protocol_;

}; 

} // kernel
