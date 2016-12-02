#pragma once 
#include "human_view.h"
#include "network/msg_dispatcher.h"
#include "objects/impl/local_position.h"
#include "kernel/systems_fwd.h"

namespace human
{
    struct visual
        : view 
    {
        static object_info_ptr create(object_create_t const& oc, dict_copt dict);

    private:
        visual(object_create_t const& oc, dict_copt dict);

    protected:
        void update     (double time);


    };

}