#pragma once

namespace shaders
{
    enum shader_t {VS,TES,TCS,GS,FS};

    namespace plane_mat
    {
        const char* get_shader(shader_t t);
    }

    namespace default_mat
    {
        const char* get_shader(shader_t t);
    }  

    namespace building_mat
    {
        const char* get_shader(shader_t t);
    }

    namespace tree_mat
    {
        const char* get_shader(shader_t t);
    }      

    namespace ground_mat
    {
        const char* get_shader(shader_t t);
    }     
    
    namespace concrete_mat
    {
        const char* get_shader(shader_t t);
    }

    namespace railing_mat
    {
        const char* get_shader(shader_t t);
    }

    namespace panorama_mat
    {
        const char* get_shader(shader_t t);
    }

    namespace sky_fog_mat
    {
        const char* get_shader(shader_t t);
    }       

    namespace clouds_mat
    {
        const char* get_shader(shader_t t);
    } 
}