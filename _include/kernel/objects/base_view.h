#pragma once
#include "network/msg_dispatcher.h"
#include "kernel/object_info_fwd.h"
//#include "kernel/systems/history_system.h"
//#include "python/utils.h"

namespace kernel
{

//! интерфейс сохранения данных
struct obj_data_saver
{
    virtual void save_obj_data(dict_t& dict, bool key_safe) const = 0;
};

//! некий trait (?), обеспечивающий чтение и запись данных между словарем (dict) и реальными двоичными структурами объектов
//! @@@ важно
template<class derived_t>
struct obj_data_holder
    : obj_data_saver        // интерфейс
    , protected derived_t   // параметр шаблона
{
protected:
    typedef obj_data_holder<derived_t>  obj_data_base;
    typedef derived_t                   obj_data_t;

    template<class t1>                      obj_data_holder(t1 const& arg1)                                 : derived_t(arg1)               {}
    template<class t1, class t2>            obj_data_holder(t1 const& arg1, t2 const& arg2)                 : derived_t(arg1, arg2)         {}
    template<class t1, class t2, class t3>  obj_data_holder(t1 const& arg1, t2 const& arg2, t3 const& arg3) : derived_t(arg1, arg2, arg3)   {}

protected:
    explicit obj_data_holder(derived_t const& d)
    {
        obj_data() = d;
    }

    explicit obj_data_holder(dict_cref dict, bool fail_on_absence = false)
    {
        load_obj_data(dict, fail_on_absence);
    }

    explicit obj_data_holder(dict_copt dict, bool fail_on_absence = false)
    {
        if (dict)
            load_obj_data(*dict, fail_on_absence);
    }

    derived_t&       obj_data()       { return static_cast<derived_t&>      (*this); }
    derived_t const& obj_data() const { return static_cast<derived_t const&>(*this); } 

private:
    //! для чтения данных из словаря в двоичную предметно-ориентированную структуру
    void load_obj_data(dict_cref dict, bool fail_on_absence)
    {
        derived_t& obj = static_cast<derived_t&>(*this);

        if (dict.data().empty())
            read(dict, obj, fail_on_absence);
        else 
            binary::unwrap(dict.data(), obj);
    }

    // для записи данных в словарь из двоичной предметно-ориентированной структуры
    void save_obj_data(dict_t& dict, bool key_safe) const override
    {
        derived_t const& obj = static_cast<derived_t const&>(*this);

        if (key_safe)
            write(dict, obj);
        else 
            dict.data() = binary::wrap(obj);
    }
};

//! обертка для хранения "настроек" чего-либо в виде структуры, с рефлексией
//! @@@ важно
template<class settings_t>
struct wrap_settings
{
    wrap_settings()
    {
    }

    wrap_settings(settings_t const& st) // implicit
        : settings_(st)
    {
    }

protected:
    settings_t const& settings() const { return settings_; }
    settings_t&       settings()       { return settings_; }

protected:
    settings_t settings_;

    REFL_INNER(wrap_settings)
        REFL_ENTRY(settings_)
    REFL_END()
};

//////////////////////////////////////////////////////////////////////////
// base view

//! базовый класс для любого "представления" 
struct /*SYSTEMS_API*/ base_view_presentation
    : kernel::base_presentation // базовый интерфейс представления
    //, kernel::history_prs     // что-то связанное с историей // now history is generic for all objects
{
    base_view_presentation(kernel::object_create_t const& oc);

// object_info
public:
    string const&                       name     () const;
    void                                set_name (std::string const& name);   // a workaround for displaying name in attributes widget
    uint32_t                            object_id() const;
    kernel::object_info_wptr            parent   () const;
    kernel::object_info_vector const&   objects  () const;
    kernel::object_class_ptr            hierarchy_class() const;
    void                                save     (dict_ref dict, bool key_safe) const;

public:
    DECLARE_EVENT(state_modified, ());

public:
    //! держатель соединений
    connection_holder& conn_holder();

protected:
    template<class msg_t>
    void set(msg_t const& msg, bool sure = true)
    {
//     Information about obj_2 changing on presentation_1 should spread to obj_1 presentation_2 
//     only by way (3) -> (4), not by way (1) -> (2). But notification (1) should work as well. 
// 
//     So we block ability to send messages (2) while obj_2 invokes 'set' function to renew its state  
        
//       -----------   event  -----------
//      [ obj1 prs1 ] <-(1)- [ obj2 prs1 ] 
//       -----------       /  -----------
//           |            /       |
//           |           /        |
//         msg (2)     (?)       msg (3)
//           |         /          |
//           |        /           |
//           V       /            V
//      ------------L  event  -----------
//      [ obj1 prs2 ] <-(4)- [ obj2 prs2 ] 
//       -----------          -----------
                                                       
        block_msgs_(true);
        {
            // most notification are fired from this handler 
            msg_disp().on_msg(msg);
            state_modified_signal_();
        }
        block_msgs_(false);

        send(msg, sure);        
    }

    template<class msg_t>
    void send_cmd(msg_t const& msg, bool sure = true)
    {
        msg_disp().on_msg(msg);
        send(msg, sure, true);
    }

protected:
    void send_msg(binary::bytes_cref bytes, bool sure, bool just_cmd);

// tree_object
protected:
    void append_child( kernel::object_info_ptr object );
    void remove_child( kernel::object_info_ptr object );

// base_presentation
protected:
    void pre_update (double time) override;
    void update     (double time) override;
    void post_update(double time) override;
    void update_atc (double time) override;
    void on_msg     (binary::bytes_cref   bytes )  override;
    void reset_parent (kernel::object_info_wptr parent)  override;

// history_prs
private:
    network::msg_dispatcher<>::ids_t msg_ids   () const;
    uint32_t                         msg_sub_id(size_t id, binary::bytes_cref data) const;

private:
    template<class msg_t>
    void send(msg_t const& msg, bool sure = true, bool just_cmd = false)
    {
        auto data = network::wrap_msg(msg);
        send_msg(data, sure, just_cmd);
    }

private:
    void child_appended(kernel::object_info_ptr /*child*/);
    void child_removing(kernel::object_info_ptr /*child*/);
    void parent_changed();

    void object_created   (kernel::object_info_ptr /*object*/);
    void object_destroying(kernel::object_info_ptr /*object*/);

// own interface
protected: 
    virtual void on_child_appended(kernel::object_info_ptr /*child*/) {}
    virtual void on_child_removing(kernel::object_info_ptr /*child*/) {}
    virtual void on_parent_changed() {}

protected:
    virtual void on_object_created   (kernel::object_info_ptr /*object*/) {}
    virtual void on_object_destroying(kernel::object_info_ptr /*object*/) {}

protected:
    network::msg_dispatcher<>& msg_disp();

protected:
    kernel::system* const            sys_;
    kernel::object_collection* const collection_;

private:
    //! для питона
    //PY_REG_STRUCT()
    //{
    //    using namespace py;

    //    class_<base_view_presentation, boost::noncopyable>("base_view", "base view presentation", no_init)
    //        .def("name", &base_view_presentation::name, py::return_value_policy<copy_const_reference>())
    //        .def("id",   &base_view_presentation::object_id);
    //}
    // FIXME stub
    // FIXME(Питонная заглушка)
    boost::python::object py_ptr() const
    {                                   
        return boost::python::object(boost::python::ptr(this));
    }

private:
    size_t                      object_id_;
    string                      name_;
    kernel::object_info_wptr    parent_;
    kernel::object_info_vector  objects_;

    kernel::send_msg_f          send_msg_;
    kernel::block_obj_msgs_f    block_msgs_;
    kernel::object_class_ptr    hierarchy_class_;

private:
    //! диспетчер сообщений для всех объектов представлений (межмодульное, межмашинное взаимодействие)
    //! есть геттер для него; также в других классах есть несколько других таких же объектов
    network::msg_dispatcher<>   msg_disp_;

private:
    //! основной держатель соединений - объект (есть функция-геттер, используется всеми видами, наследующими от этого базового вида)
    //! тем ни менее некоторые классы не наследуются от базового вида и имеют свои держатели соединений
    connection_holder    conn_holder_;
};

typedef polymorph_ptr<base_view_presentation> base_view_presentation_ptr ;


} // objects
