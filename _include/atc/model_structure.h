#pragma once
#include "bin_data/bin_data.h"
#include "bin_data/io_streams_fwd.h"

namespace model_structure
{
struct triangles_chunk
{
    uint32_t          start_index;
    uint32_t          size;
    std::string       material;
};

struct  collision_volume
{
    std::vector<triangles_chunk>   chunks;
    std::vector<point_3i> indices;
    std::vector<point_3f> vertices;
};

typedef shared_ptr<collision_volume> collision_volume_ptr;

//FIXME("XXX")

struct node_data
{
    std::string                    name;
    cg::point_3                    pos;
    cg::quaternion                 orien;
    /*cg::sphere_3 */cg::rectangle_3                  bound;
    std::vector<std::string>       victory_nodes;
    std::vector<std::string>       node_ids;
};

struct collision_structure
{
    ph_map<string, collision_volume_ptr>::map_t collision_volumes;
};


inline cg::rectangle_3 bounding(collision_volume const& v)
{
    cg::rectangle_3 rc;
    for (size_t i = 0; i < v.chunks.size(); ++i)
    {
        for (size_t j = 0 ; j < v.chunks[i].size; ++j)
        {
            for (size_t k = 0; k < 3; ++k)
                rc |= v.vertices[v.indices[v.chunks[i].start_index + j][k]];
        }
    }

    return rc;
}

REFL_STRUCT(triangles_chunk)
    REFL_ENTRY(start_index)
    REFL_ENTRY(size       )
    REFL_ENTRY(material   )
REFL_END()

REFL_STRUCT(node_data)
    REFL_ENTRY(name         )
    REFL_ENTRY(pos          )
    REFL_ENTRY(orien        ) 
    REFL_ENTRY(bound        )
    REFL_ENTRY(victory_nodes)
    REFL_ENTRY(node_ids)
REFL_END()

REFL_STRUCT(collision_volume)
    REFL_ENTRY(chunks  )
    REFL_ENTRY(indices )
    REFL_ENTRY(vertices)
REFL_END()

inline void write(binary::output_stream& stream, model_structure::collision_structure const& data)
{
    write(stream, (binary::size_type)data.collision_volumes.size());
    for (auto it = data.collision_volumes.begin(); it != data.collision_volumes.end(); ++it)
    {
        write(stream, it->first);
        write(stream, *it->second);
    }
}

inline void read(binary::input_stream& stream, model_structure::collision_structure & data)
{
    binary::size_type size = binary::read_stream<binary::size_type>(stream);
    for (binary::size_type i = 0; i < size; ++i)
    {
        string name = binary::read_stream<string>(stream);
        model_structure::collision_volume_ptr colvol = boost::make_shared<model_structure::collision_volume>();
        read(stream, *colvol);
        data.collision_volumes.insert(std::make_pair(name, colvol));
    }
}

} // model_structure
