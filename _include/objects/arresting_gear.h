#pragma once


namespace arresting_gear
{
    struct rope_node_state_t
    {
        rope_node_state_t(): on(false){}
        rope_node_state_t( cg::point_3 const& vel, cg::point_3 const& coord)
            : vel(vel), coord(coord)
        {}

        cg::point_3 vel;
        cg::point_3 coord;
        bool        on;

        REFL_INNER(rope_node_state_t)
            REFL_ENTRY(vel)
            REFL_ENTRY(coord)
        REFL_END()
    };
    
    struct rope_state_t
    {
        typedef std::vector<rope_node_state_t> nodes_t;
        nodes_t nodes;
        bool    on;
         
        rope_state_t(): on(false){}
        rope_state_t(nodes_t&& nodes , bool&& on)
            : nodes  (std::move(nodes))
            , on(on)
        {
        }

        __forceinline size_t size() const
        {
            return nodes.size();
        }
         
        __forceinline nodes_t::iterator end() 
        {
            return nodes.end();
        }
         
        __forceinline  nodes_t::const_iterator end() const
        {	// return iterator for end of nonmutable sequence
            return nodes.end();
        }

        __forceinline nodes_t::iterator begin() 
        {
            return nodes.begin();
        }
             
        __forceinline  nodes_t::const_iterator begin() const
        {	// return iterator for beginning of nonmutable sequence
            return nodes.begin();
        }

        __forceinline  nodes_t::const_reference operator[](nodes_t::size_type _Pos) const
        {	// subscript
            return nodes.operator[](_Pos);
        }

        __forceinline  nodes_t::reference operator[](nodes_t::size_type _Pos) 
        {	// subscript
            return nodes.operator[](_Pos);
        }
 	    
        __forceinline   void resize(nodes_t::size_type _Newsize)
		{	
            nodes.resize(_Newsize);
		}

      	__forceinline   void reserve(nodes_t::size_type _Count)
		{	
            nodes.reserve(_Count);
		}

        REFL_INNER(rope_state_t)
            REFL_ENTRY(nodes)
            REFL_ENTRY(on)
        REFL_END()
    };

    typedef std::vector<rope_state_t>      ropes_state_t;

}