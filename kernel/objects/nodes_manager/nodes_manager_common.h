#pragma once

#include "objects/impl/local_position.h"
#include "attributes/attr_common.h"

#include "reflection/proc/binary.h"
#include "objects/nodes_management.h"

namespace nodes_management
{

// squeeze support 
enum component_type
{
    ct_pos_x    = 1 << 0,
    ct_pos_y    = 1 << 1,
    ct_pos_z    = 1 << 2,

    ct_dpos_x   = 1 << 3,
    ct_dpos_y   = 1 << 4,
    ct_dpos_z   = 1 << 5,

    ct_orien_x  = 1 << 6,
    ct_orien_y  = 1 << 7,
    ct_orien_z  = 1 << 8,

    ct_omega_x  = 1 << 9,
    ct_omega_y  = 1 << 10,
    ct_omega_z  = 1 << 11,

    ct_loc_glob = 1 << 12,

    ct_pos      = ct_pos_x   | ct_pos_y   | ct_pos_z  ,
    ct_dpos     = ct_dpos_x  | ct_dpos_y  | ct_dpos_z ,
    ct_orien    = ct_orien_x | ct_orien_y | ct_orien_z,
    ct_omega    = ct_omega_x | ct_omega_y | ct_omega_z,

    ct_none     = 0,
    ct_all      = ct_pos | ct_dpos | ct_orien | ct_omega | ct_loc_glob,
};

#pragma pack(push, 1)

struct node_pos
{
    node_pos(node_position const& p)
    {
        if (p.is_local())
        {
            auto const& lp = p.local();
            pos     = lp.pos;
            dpos    = lp.dpos;
            orien   = lp.orien.rot_axis().omega();
            omega   = lp.omega;

            loc = in_place(lp.relative_obj, lp.relative_node);
        }
        else 
        {
            auto const& gp = p.global();
            pos     = point_3(gp.pos.lat, gp.pos.lon, gp.pos.height);
            dpos    = gp.dpos;
            orien   = gp.orien.rot_axis().omega();
            omega   = gp.omega;
        }
    }

    node_pos()
    {
    }

    double const*   as_doubles() const  { return reinterpret_cast<double const*>(this); }
    double*         as_doubles()        { return reinterpret_cast<double*      >(this); }
    static size_t   doubles_count()     { return 12; }

    node_position get_pos() const 
    {
        if (loc)
        {
            local_position lp;
            lp.pos  = pos;
            lp.dpos = dpos;
            lp.orien= quaternion(rot_axis(orien));
            lp.omega= omega;

            lp.relative_obj = loc->rel_obj;
            lp.relative_node= loc->rel_node;

            return lp;
        }
        else 
        {
            geo_position gp;

            gp.pos  = geo_point_3(pos.x, pos.y, pos.z);
            gp.dpos = dpos;
            gp.orien= quaternion(rot_axis(orien));
            gp.omega= omega;

            return gp;
        }
    }

    node_position get_pos(node_position const& base, uint16_t components) const 
    {
        node_pos res(base);

        if ((components & ct_loc_glob) == ct_loc_glob)
            res.loc = loc;

        auto resd = res.as_doubles();
        auto myd  = as_doubles();

        for (size_t i = 0, count = node_pos::doubles_count(); i != count; ++i)
        {
            uint16_t component = 1 << i;
            if ((component & components) == component)
                resd[i] = myd[i];
        }

        return res.get_pos();        
    }

    point_3 pos;
    point_3 dpos;
    point_3 orien;
    point_3 omega;
    
    struct rel_pos
    {
        rel_pos()
            : rel_obj (-1)
            , rel_node(-1)
        {
        }

        rel_pos(uint32_t rel_obj, uint32_t rel_node)
            : rel_obj (rel_obj)
            , rel_node(rel_node)
        {
        }

        bool operator==(rel_pos const& rp) const
        {
            return  rel_obj  == rp.rel_obj && 
                    rel_node == rp.rel_node;
        }

        uint32_t rel_obj;
        uint32_t rel_node;

        REFL_INNER(rel_pos)
            REFL_SER(rel_obj)
            REFL_SER(rel_node)
        REFL_END()
    };

    optional<rel_pos> loc;
};

#pragma pack(pop)

//

namespace msg 
{

enum 
{
    nm_model,
    nm_node_data,
};

enum node_msg_id
{
    nm_node_pos         ,
    nm_freeze_state     ,
    nm_node_animation   ,
    nm_node_visibility  ,
    nm_node_texture     ,
};

struct model_msg
    : network::msg_id<nm_model>
{
    model_msg(string const& model = "")
        : model(model)
    {
    }

    string model;
};


struct node_data
    : network::msg_id<nm_node_data>
{
    typedef uint16_t node_id_t;

    node_data(){}
    node_data(node_id_t node_id, bytes_cref data) : node_id(node_id), data(data){}
    node_data(node_id_t node_id, bytes_t&&  data) : node_id(node_id), data(move(data)){}

    node_id_t   node_id;
    bytes_t     data;
};

//////////////////////////////////////////////////////////////////////////
// nodes messages

struct node_pos_descr
{
    node_pos_descr(optional<double> const& time, uint16_t components, node_pos const& pos)
        : time          (time   )
        , components    (components)
        , pos           (pos    )
    {
    }

    node_pos_descr()
        : components(ct_none)
    {
    }

    optional<double> time;
    uint16_t         components;
    node_pos         pos;
};


typedef gen_msg<nm_freeze_state , node_pos_descr> freeze_state_msg;
typedef gen_msg<nm_node_pos     , node_pos_descr> node_pos_msg;

struct node_animation
    : network::msg_id<nm_node_animation>
{
    node_animation() {}
    node_animation(double time, string const& name, float len, float from, float size, float cross_fade);

    double  time;
    string  name;
    float   len;
    float   from;
    float   size;
	float   cross_fade;
};

struct visibility_msg
    : network::msg_id<nm_node_visibility>
{
    visibility_msg(bool visible = false)
        : visible(visible)
    {
    }

    bool visible;
};

struct node_texture_msg
    : network::msg_id<nm_node_texture>
{
    node_texture_msg(string const& tex = "")
        : tex       (tex)
    {
    }

    string tex;
};

REFL_STRUCT(model_msg)
    REFL_ENTRY(model)
REFL_END()

REFL_STRUCT(node_data)
    REFL_ENTRY(node_id)
    REFL_ENTRY(data)
REFL_END   ()

void write(binary::output_stream& os, node_pos_descr const& msg);
void read (binary::input_stream & is, node_pos_descr& msg);

REFL_STRUCT(visibility_msg)
    REFL_ENTRY(visible)
REFL_END()

REFL_STRUCT(node_animation)
    REFL_ENTRY(time)
    REFL_ENTRY(name)
    REFL_ENTRY(len )
    REFL_ENTRY(from)
    REFL_ENTRY(size)
	REFL_ENTRY(cross_fade)
REFL_END()


REFL_STRUCT(node_texture_msg)
    REFL_ENTRY(tex)
REFL_END()

// ----------------------------------------------------------------------------- impl
//

inline void write(binary::output_stream& os, node_pos_descr const& msg)
{
    write(os, char(msg.time.is_initialized()));

    if (msg.time)
        write(os, msg.time.get());

    write(os, msg.components);

    if ((msg.components & ct_loc_glob) == ct_loc_glob)
        write(os, msg.pos.loc);

    auto doubles = msg.pos.as_doubles();
    for (size_t i = 0, count = node_pos::doubles_count(); i != count; ++i)
    {
        uint16_t component = 1 << i;
        if ((component & msg.components) == component)
        {
            if (msg.pos.loc || component > ct_pos_y)
                write(os, float(doubles[i]));
            else 
                write(os, doubles[i]); // lat and lon of geo_point
        }
    }
}

inline void read(binary::input_stream& is, node_pos_descr& msg)
{
    if (read_stream<char>(is))
        msg.time = read_stream<double>(is);

    read(is, msg.components);

    if ((msg.components & ct_loc_glob) == ct_loc_glob)
        read(is, msg.pos.loc);

    auto doubles = msg.pos.as_doubles();
    for (size_t i = 0, count = node_pos::doubles_count(); i != count; ++i)
    {
        uint16_t component = 1 << i;
        if ((component & msg.components) == component)
        {
            if (msg.pos.loc || component > ct_pos_y)
                doubles[i] = read_stream<float>(is);
            else 
                read(is, doubles[i]); // lat and lon of geo_point
        }
    }
}

//

inline node_animation::node_animation(double time, string const& name, float len, float from, float size, float cross_fade)
    : time(time)
    , name(name)
    , len (len )
    , from(from)
    , size(size)
	, cross_fade(cross_fade)
{}

} // msg
} // nodes_management 