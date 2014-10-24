#pragma once

namespace shaders
{
    enum shader_t {VS,GS,FS};
    namespace plane_mat
    {
        const char* get_shader(shader_t t);
    }

    namespace default_mat
    {
        const char* get_shader(shader_t t);
    }
}