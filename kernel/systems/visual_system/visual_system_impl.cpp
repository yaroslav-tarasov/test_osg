#include "visual_system_impl.h"
#include "visual_object_impl.h"
#if 0
#include "nfi/nfi.h"
#endif

#include "geometry/frustum.h"


namespace kernel
{
namespace details
{
    template< class I >
    struct cyclic_iterator
        /* : public iterator< bidirectional, yadda yadda > */ 
    {
        typedef typename I::value_type value_type;

        cyclic_iterator( I f, I l, size_t c )
            : it(f), beg(f), end(l), cnt(c)
        {}

        cyclic_iterator() : it(), beg(), end(), cnt() 
        {}

        cyclic_iterator &operator++() 
        {
            ++it;
            if (it == end)
            {
                ++cnt;
                it = beg;
            }

            return *this;
        }

        friend bool operator== ( cyclic_iterator const &lhs, cyclic_iterator const &rhs )
        { 
            return lhs.it == rhs.it && lhs.cnt == rhs.cnt; 
        } 

        friend bool operator!= ( cyclic_iterator const &lhs, cyclic_iterator const &rhs )
        {
            return !(lhs == rhs);
        }

        value_type operator*()
        {
            return *it;
        }

    private:
        I it, beg, end;
        size_t cnt;
    };

    template<class I>
    pair< cyclic_iterator<I>, cyclic_iterator<I> > cycle_range( I f, I l, int c = 1 )
    {
        return make_pair(cyclic_iterator<I>(f, l, 0), cyclic_iterator<I>(f, l, c));
    }

}





visual_system_impl::visual_system_impl(msg_service& service, av::IVisualPtr vis, vis_sys_props const& props)
	: system_base(sys_visual, service, "objects.xml")
	, vis_   (vis)
	, scene_ (vis->GetScene())
	, ready_ (false)
	//, viewport_ (vis->create_viewport())

	, props_(props)
	, object_destroying_connection_(this->subscribe_object_destroying(boost::bind(&visual_system_impl::object_destroying, this, _1)))
	, free_cam_(false)
{
	LogInfo("Create Visual Subsystem");
	vis_->SetFreeViewControl(this);
}

av::IVisualPtr   visual_system_impl::visual()
{
	return  vis_;
}

av::IScenePtr    visual_system_impl::scene()
{
	return scene_;
}

void visual_system_impl::update(double time)
{
	system_base::update(time);

	// scene_->update(time);
	if(!free_cam_)
		update_eye();
}

vis_sys_props const& visual_system_impl::vis_props() const
{
	return props_;
}

void visual_system_impl::update_props(vis_sys_props const& props)
{
	props_ = props;
	init_eye();
}

void visual_system_impl::load_exercise(dict_cref dict)
{
	system_base::load_exercise(dict);
	init_eye();
}

visual_object_ptr visual_system_impl::create_visual_object( std::string const & res, on_object_loaded_f f, uint32_t seed/* = 0*/, bool async )
{
#if 0
	LogInfo("Objects to create: " << res << " seed = " << seed);
#endif
	if (seed>0 && !ready_)
	{
		objects_to_create_.insert(seed);
	}

	return boost::make_shared<visual_object_impl>( res, seed, async, [this,f](uint32_t seed)->void {  f(seed); this->visual_object_created(seed); });
}

visual_object_ptr visual_system_impl::create_visual_object( nm::node_control_ptr parent,std::string const & res, on_object_loaded_f f, uint32_t seed/* = 0*/, bool async )
{
#if 0
	LogInfo("Objects to create: " << res << " seed = " << seed);
#endif
	if (seed>0 && !ready_)
	{
		objects_to_create_.insert(seed);
	}

	return boost::make_shared<visual_object_impl>( parent, res, seed, async,  [this,f](uint32_t seed)->void {  f(seed); this->visual_object_created(seed); });
}

void    visual_system_impl::visual_object_created( uint32_t seed )
{
	auto it = objects_to_create_.find(seed);
	if(it!=objects_to_create_.end())
		objects_to_create_.erase(it);

#if 0
	for (auto it = objects_to_create_.begin(); it!= objects_to_create_.end(); ++it)
		LogInfo("Objects left to create: " << " seed = " << *it);
#endif
	usual_objects_to_load_--;

	if(usual_objects_to_load_ == 0 && !ready_  )  
	{
		exercise_loaded_signal_();
		ready_ = true;
	}

#if 0
	LogInfo("Objects left to create: " << objects_to_create_.size() << " seed = " << seed);
#endif
}

void visual_system_impl::init_eye()
{
	// no eye is selected if camera name is incorrect to show problem visually!!!
	// please don't change selection logic
#if 0
	eye_ = (!props_.channel.camera_name.empty()) 
		? find_object<visual_control_ptr>(this, props_.channel.camera_name)
		: find_first_object<visual_control_ptr>(this) ;

	if (!eye_ && !props_.channel.camera_name.empty())
		LogWarn("Can't find camera: " << props_.channel.camera_name);

#else
	eye_ = find_object<visual_control_ptr>(this, props_.channel.camera_name);

	if (!eye_ && !props_.channel.camera_name.empty())
		LogWarn("Can't find camera: " << props_.channel.camera_name);

	if(!eye_)
		eye_ = find_first_object<visual_control_ptr>(this) ;
#endif


#if 0
	viewport_->SetClarityScale(props_.channel.pixel_scale);
	viewport_->set_geom_corr(props_.channel.cylindric_geom_corr ? victory::IViewport::explicit_cylinder : victory::IViewport::no_geom_corr);

	init_frustum_projection();
#endif

	update_eye();

}

void visual_system_impl::update_eye()
{
	const auto & cam = /*debug_eye_ ? debug_eye_camera() :*/ eye_camera();
	//viewport_->SetPosition(cam.position(), cam.orientation());
	vis_->SetPosition(cam.position(), cg::quaternion(cam.orientation()));
}

cg::camera_f visual_system_impl::eye_camera() const
{
	typedef cg::rotation_3f rot_f;

	cg::camera_f cam = eye_ 
		? cg::camera_f(point_3f(geo_base_3(props_.base_point)(eye_->pos())), cprf(eye_->orien()))
		: cg::camera_f(point_3f(0,0,100), cprf(-90));

	cam.set_orientation((rot_f(cam.orientation()) * rot_f(cprf(props_.channel.course))).cpr());
	return cam;
}

void visual_system_impl::object_destroying(object_info_ptr object)
{
	if (eye_ == object)
		eye_.reset();
	//else if (debug_eye_ && debug_eye_->track_object == object)
	//    debug_eye_.reset();
}


}


