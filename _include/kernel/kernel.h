#pragma once

#include "kernel/systems_fwd.h"
#include "kernel/object_info_fwd.h"
#include "systems/systems_base.h"

//! структура для создания объектов

namespace kernel
{
    //! исключение на случай отсутствия объекта (похоже не используется??)
    struct presentation_failed 
        : std::exception
    {
    };


    //! информация для создания объектов (из классов библиотеки objects); 
    struct object_create_t
    {
        object_create_t(
            object_class_ptr                hierarchy_class , 
            kernel::system*                 sys             , 
            size_t                          object_id       , 
            string const&                   name            , 
            vector<object_info_ptr> const&  objects         , 
            kernel::send_msg_f const&       send_msg        , 
            kernel::block_obj_msgs_f        block_msgs      ,
            object_data_t                   object_data
            )

            : hierarchy_class   (hierarchy_class)
            , sys               (sys            )
            , object_id         (object_id      )
            , name              (name           )
            , objects           (objects        )
            , send_msg          (send_msg       )
            , block_msgs        (block_msgs     )
            , object_data       (object_data    )
        {}

        object_class_ptr                hierarchy_class;
        kernel::system*                 sys;
        size_t                          object_id;
        string                          name; 
        kernel::send_msg_f              send_msg;
        kernel::block_obj_msgs_f        block_msgs;
        std::vector<object_info_ptr>    objects;
        object_data_t                   object_data;
    };
}