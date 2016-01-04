#pragma once

#include "geometry/clipping/clip_data.h"


namespace av
{

namespace part_sys
{

#if 1
    template <class GPUParticle>
    struct gpu_part_drawer
    {
        gpu_part_drawer() : /*gd_(vtGlobal::pGD), */parts_(0) {}

#if 0
        __forceinline void set_vertex_format( const vtVertexFormat & vf )
        {
            const RenderContextLocker rcl_lock;

            // vbo
            IBufferObjectPtr vbo = gd_->CreateBuffer();
            vbo->SetTarget(GL_ARRAY_BUFFER);
            vbo->Data(sizeof(GPUParticle), nullptr);
            // vao
            vao_ = gd_->CreateVAO();
            vao_->SetVertexVBO(vbo.get());
            vao_->SetVertexFormat(vf);
            vao_->Actualize();
        }
#endif

        //template<typename CPU_Queue, typename GPU_PartType>
        //__forceinline unsigned fill( CPU_Queue const & cpu_queue, std::function<void (GPU_PartType &, typename CPU_Queue::particle_type_t const &)> const & fill_proc )
        template<typename CPU_Queue, typename GPU_PartType, typename Func>
        __forceinline unsigned fill( CPU_Queue const & cpu_queue, cg::aabb_clip_data & aabb, float part_max_size, Func const & fill_proc )
        {
            parts_ = cpu_queue.size();
            if (!parts_)
                return 0;

            // cpu storage for gpu particles
            static std::vector<GPU_PartType> particles_storage;
            particles_storage.resize(parts_);

            // scan all active and fill gpu queue
            cg::rectangle_3f result_aabb;
            auto cur_gpu_part = &particles_storage.front();
            for (auto part = cpu_queue.begin(), it_end = cpu_queue.end(); part != it_end; ++part)
            {
                fill_proc(*(cur_gpu_part++), *part);
                result_aabb |= part->cur_pos();
            }

            // extend and save aabb
            if (!result_aabb.empty())
                result_aabb.inflate(part_max_size);
            aabb = result_aabb.empty() ? cg::aabb_clip_data() : cg::aabb_clip_data(result_aabb);

#if 0
            const RenderContextLocker rcl_lock;

            // feed it to GPU
            auto vbo = vao_->GetVertexVBO();
            auto data_size = parts_ * sizeof(GPUParticle);
            vbo->Data(data_size, nullptr);
            vbo->SubData(0, data_size, &particles_storage.front());
#endif

            return parts_;
        }

        __forceinline void draw() const
        {
            if (parts_)
            {
#if 0
                gd_->BindVAO(vao_.get());
                gd_->DrawArrays(GL_POINTS, 0, parts_);
#endif
            }
        }

        __forceinline unsigned size() const
        {
            return parts_;
        }

    private:

#if 0
        IGraphicDevice * gd_;
        IVertexArrayObjectPtr vao_;
#endif
        unsigned parts_;
    };
#endif


} // namespace part_sys

} // namespace av
