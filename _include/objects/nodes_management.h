#pragma once

#include "nodes_management_fwd.h"
#include "cpp_utils/polymorph_ptr.h"
#include "atc/model_structure.h"

	namespace nodes_management
	{
	struct settings_t
	{
		settings_t()
			: model("checker")
		{}
		settings_t(std::string const& model)
			: model(model)
		{}

		std::string model;
	};

	REFL_STRUCT(settings_t)
		REFL_ENTRY(model)
	REFL_END()

#if 0
    struct node_position
    {
        node_position() : pos_(geo_position()) { }
        node_position( geo_position const& pos ) : pos_(pos) { }
        node_position( local_position const& pos ) : pos_(pos) { }

        geo_position const & global() const { return boost::any_cast<geo_position const&>(pos_); }
        geo_position       & global()       { return boost::any_cast<geo_position &>(pos_); }

        local_position const & local() const { return boost::any_cast<local_position const&>(pos_); }
        local_position       & local()       { return boost::any_cast<local_position&>(pos_); }

        bool is_local() const { return typeid(local_position) == pos_.type(); }

        void set_local(local_position const& p) { pos_ = p; }
        void set_global(geo_position const& p) { pos_ = p; }

        bool is_static() const;
        void set_static();
           
        template<class processor> friend void reflect(processor&, node_position const&);

    private:
        boost::any pos_;
    };
#else
    struct node_position
    {
        node_position() : pos_(geo_position()) { }
        node_position( geo_position const& pos ) : pos_(pos) { }
        node_position( local_position const& pos ) : pos_(pos) { }

        geo_position const & global() const { return boost::get<geo_position>(pos_); }
        geo_position       & global()       { return boost::get<geo_position>(pos_); }

        local_position const & local() const { return boost::get<local_position>(pos_); }
        local_position       & local()       { return boost::get<local_position>(pos_); }

        bool is_local() const { return typeid(local_position) == pos_.type(); }

        void set_local(local_position const& p) { pos_ = p; }
        void set_global(geo_position const& p)  { pos_ = p; }

        bool is_static() const;
        void set_static();

        template<class processor> friend void reflect(processor&, node_position const&);

    private:
        boost::variant<local_position,geo_position> pos_;
    };
#endif
REFL_STRUCT(node_position)
    
    bool __local__ = obj.is_local();
    REFL_SER_RAW(__local__)

    if (__local__)
        REFL_SER_AS_TYPE(pos_, local_position)
    else 
        REFL_SER_AS_TYPE(pos_, geo_position)
REFL_END()

    //! вероятно некая проверка на начлаьные значения полей 'dpos' и 'omega'
    inline bool node_position::is_static() const
    {
        if (is_local())
            return local ().dpos == cg::point_3() && local ().omega == cg::point_3();
        else
            return global().dpos == cg::point_3() && global().omega == cg::point_3();
    }

    //! (не исп.)???
    inline void node_position::set_static()
    {
        if (is_local())
        {
			//local().dpos = point_3();
			//local().omega = point_3();
			local().freeze();
        }
        else
        {
			//global().dpos = point_3();
			//global().omega = point_3();
			global().freeze();
        }
    }

    struct node_info
    {
        virtual ~node_info(){}

        virtual node_position const&         position   () const = 0;
        virtual transform_4                  transform  () const = 0;
        virtual uint32_t                     node_id    () const = 0;
        virtual uint32_t                     object_id  () const = 0;
        virtual std::string const&           name       () const = 0;
                                                                                    
        virtual node_info_ptr                rel_node()    const = 0;
        virtual node_info_ptr                root_node()   const = 0;   // FIXME не всякий рут можно найти

        virtual cg::transform_4              get_root_transform() const = 0;
        virtual cg::geo_point_3              get_global_pos()     const = 0;
        virtual cg::quaternion               get_global_orien()   const = 0;

        virtual model_structure::collision_volume const* get_collision() const = 0;
        virtual /*cg::sphere_3*/ cg::rectangle_3                get_bound() const = 0;
    };


    struct node_control
        : node_info
    {
        virtual ~node_control(){}

        virtual void set_position    (node_position const& pos) = 0;
        virtual void play_animation  (std::string const& seq, double len, double from, double size, double cross_fade) = 0;
        virtual void set_texture     (std::string const& texture) = 0;
        virtual void set_visibility  (bool visible) = 0;
        virtual boost::optional<bool> get_visibility  () = 0;
    };

	struct node_tree_iterator
	{
		virtual ~node_tree_iterator(){}

		virtual std::vector<node_tree_iterator_ptr> const& children() const = 0 ;
		virtual node_info_ptr node() const = 0 ;
	};

	struct vis_node_control
	{
		typedef 
			std::vector</*victory::node_ptr*/osg::ref_ptr<osg::Node>> 
			vis_nodes_t;

		virtual ~vis_node_control(){}
		virtual vis_nodes_t const& vis_nodes() const = 0;
		virtual bool is_visible() const = 0;
        virtual void set_visibility  (bool visible) = 0;
	};

	struct vis_node_info
	{
		virtual ~vis_node_info() {}

		virtual bool is_visible() const = 0 ;
	};

    struct manager
    {
        virtual ~manager(){}

        virtual node_info_ptr   get_node    (uint32_t node_id)   const  = 0;
        virtual node_info_ptr   find_node   (std::string const& name) const  = 0;
    
        // set_model fully reinitialize model and reset positions of all nodes and root
    
        virtual void            set_model   (string const& model, bool save_root_pos = true)  = 0;
        virtual string const&   get_model   () const                    = 0;
        virtual void            visit_nodes (boost::function<void(node_info_ptr)> const& f) const = 0;

        virtual node_tree_iterator_ptr get_node_tree_iterator(uint32_t node_id) const = 0;
        virtual  cg::transform_4       get_relative_transform( node_info_ptr node, node_info_ptr rel = nullptr) = 0;


        DECLARE_EVENT(model_changed, ());
    };


    void visit_sub_tree(node_info_ptr root, std::function<bool(node_info_ptr)> f);

    inline void visit_sub_tree(node_tree_iterator_ptr node, boost::function<bool(node_info_ptr)> f)
    {
        std::queue<node_tree_iterator_ptr> q;
        q.push(node);

        while(!q.empty())
        {
            node_tree_iterator_ptr n = q.front();
            q.pop();

            if (!f(n->node()))
                return;

            for(auto it = n->children().begin(); it != n->children().end(); ++it)
                q.push(*it);
        }
    }


    // Перенесено в менеджер
    //cg::transform_4 get_relative_transform(manager_ptr manager, node_info_ptr node, node_info_ptr rel = nullptr);


} //nm namespace