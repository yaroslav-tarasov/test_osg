#pragma once

namespace mdd
{
    struct info
    {
        virtual ~info() {}

        virtual debug_render_ptr get_renderer() = 0;
    };

    typedef polymorph_ptr<info> info_ptr;
}