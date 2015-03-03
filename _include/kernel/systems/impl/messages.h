#pragma once
#include "network/msg_base.h"

namespace kernel
{

namespace msg 
{
using namespace binary;

//! типы сообщений
enum id 
{
    sm_object_created,  // объект создан
    sm_destroy_object,  // уничтожение объекта
    
    sm_object_msg,      // сообщение объекта
    sm_container_msg,   // сообщение контейнера (?)
};

//! сообщение "объект создан"
struct object_created
    : network::msg_id<sm_object_created>
{
    bytes_t data;

    object_created()
    {
    }

    object_created(bytes_t&& data)
        : data  (move(data))
    {
    }
};

//! сообщение "уничтожение объекта"
struct destroy_object
    : network::msg_id<sm_destroy_object>
{
    size_type obj_id;

    destroy_object(size_type obj_id = size_type(-1))
        : obj_id(obj_id)
    {
    }
};

namespace details
{
    typedef uint64_t msg_obj_id_t;
    
    //! получаем идентификатор объекта из 64-битного составного идентфикатора
    inline uint32_t get_obj_id(msg_obj_id_t id) { return uint32_t((id >> 32) & 0xffffffff); }
    //! получаем идентификатор сообщения из 64-битного составного идентфикатора
    inline uint32_t get_msg_id(msg_obj_id_t id) { return uint32_t(id & 0xffffffff); }
    //! строим составной 64-битный идентификатор из идентификаторов объекта и сообщения
    inline msg_obj_id_t make_msg_obj_id(obj_id_t obj, uint32_t mesg) 
    {
        return (uint64_t(obj) << 32) + uint64_t(mesg);
    }
} // details

//! сообщение "сообщение объекта"
struct object_msg
    : network::msg_id<sm_object_msg>
{
    typedef set<details::msg_obj_id_t> msg_protocol_t;

    object_msg()
        : object_id(size_type(-1))
    {
    }

    object_msg(size_type object_id, bytes_cref data, bool just_cmd, msg_protocol_t&& prot)
        : object_id     (object_id)
        , data          (data)
        , just_cmd      (just_cmd)
        , msg_protocol_ (move(prot))
    {
    }

    size_type       object_id;
    bytes_t         data;
    bool            just_cmd;
    msg_protocol_t  msg_protocol_;
};

//! сообщение "сообщение контейнера"
struct container_msg
    : network::msg_id<sm_container_msg>
{
    //! вектор векторов байт
    typedef std::vector<bytes_t>  msgs_t;

    container_msg(){}
    
    container_msg(msgs_t&& msgs)
        : msgs(move(msgs))
    {
    }

    msgs_t msgs;
};

REFL_STRUCT(object_created)
    REFL_ENTRY(data)
REFL_END()

REFL_STRUCT(destroy_object)
    REFL_ENTRY(obj_id)
REFL_END   ()

REFL_STRUCT(object_msg)
    REFL_ENTRY(object_id)
    REFL_ENTRY(data)
    REFL_ENTRY(just_cmd)
    REFL_ENTRY(msg_protocol_)
REFL_END()

REFL_STRUCT(container_msg)
    REFL_ENTRY(msgs)
REFL_END()

} // msg
} //kernel