#pragma once 


namespace visual_objects
{
    struct parashute_support
    {

        SAFE_BOOL_OPERATOR(visual_object_)
        
        parashute_support(visual_object_ptr obj )
            : visual_object_(obj)
        {
            init_();
        }
        
        inline void update( cg::polar_point_3f const  &dir, point_3f offset )
        {
            update_(dir,offset); 
        }

 		visual_object_ptr get() const
		{
			return visual_object_;
		}

		visual_object * operator->() { return visual_object_.get();}
		visual_object const* operator->() const { return visual_object_.get();}

    private:

        inline void init_()
        {
        }


        inline void update_(cg::polar_point_3f const  &dir, point_3f offset )
        {
        }

        visual_object_ptr     visual_object_;
        nm::node_info_ptr     parent_;
    };

}