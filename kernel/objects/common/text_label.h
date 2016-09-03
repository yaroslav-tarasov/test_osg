#pragma once 

#include "common/unicode_support.h"
#include "common/labels_management.h"
#include "common/visual_objects_support_fwd.h"

namespace visual_objects
{
    struct label_support : labels_management::label_provider
    {
        
		friend  struct label_support_proxy;

		SAFE_BOOL_OPERATOR(visual_object_)

        label_support(visual_object_ptr obj,const std::string& text )
            : visual_object_(obj)
            , label_            (nullptr) 
        {
            init_(text);
        }

        inline void update( cg::polar_point_3f const  &dir, point_3f offset )
        {
            update_(dir,offset); 
        }

        visual_object_ptr get() const
        {
            return visual_object_;
        }

        inline void set_text(const std::string&  text)
        {
            const auto& geode = label_->asGeode();

            for(int i=0;i<geode->getNumDrawables(); ++i)
            {
                if(auto dtext =  dynamic_cast<osgText::Text*>(geode->getDrawable(i)))
                {
#if 0
                    std::string t("Текст");
					std::wstring wstr(t.begin(), t.end());
					std::wstring wstr2 = unicode::a2w(t).c_str();
					std::wstring wstrt(L"hhhhhhТекст");
					dtext->setText("Текст", osgText::String::ENCODING_UTF8);
#endif					
					dtext->setText(unicode::a2w(text).c_str());
                }
            }
        }

    private:

        inline void init_(const std::string&  text)
        {
            label_ = findFirstNode((visual_object_)->root(),"text_label");

            osg::ComputeBoundsVisitor cbvs;
            label_->accept( cbvs );
            const osg::BoundingBox bb_s = cbvs.getBoundingBox();

            set_text(text) ;
        }


        inline void update_(cg::polar_point_3f const  &dir, point_3f offset )
        {
        }

        //
        //  labels_management::label_provider
        //

        labels_management::labels_manager * get_labels_manager()    const  override
        {
              return labels_man_;
        }
        
        void set_labels_manager( labels_management::labels_manager * man  )     override
        {
            labels_man_ = man;
        }

        void set_font( std::string const & , unsigned ) override
        {
        }
        
        virtual void set_visible (bool v) override
        {
            visual_object_->set_visible(v);
        }

        virtual bool get_visible () override
        {
             return visual_object_->get_visible();
        }

    private:
        double                               radius_;
        osg::Node *                          label_;

        visual_object_ptr                    visual_object_;
        labels_management::labels_manager *  labels_man_;
    };
    
    struct label_support_proxy : labels_management::label_provider
    {
		label_support_ptr  get_ls() const
        {          
             return ls_;
        }

 		label_support_proxy& operator= ( label_support_ptr const& rhs )
		{
			 ls_ = rhs;
			 init_();
			 return *this;
		}

        inline void set_text(const std::string&  text)
        {
			 ls_->set_text(text);
        }

    private:
        void set_labels_manager( labels_management::labels_manager * man  )     override
        {
            labels_man_ = man;

			if(ls_)
				ls_->set_labels_manager(labels_man_);
        }
		
		labels_management::labels_manager * get_labels_manager()    const  override
		{
			if(ls_)
				return ls_->get_labels_manager();

			return labels_man_;
		}

        void set_font( std::string const & name  , unsigned size )  override
        {
            if(ls_)
				ls_->set_font(name, size);

			name_ = name;
			size_ = size;
 
        }

        virtual void set_visible (bool v) override
        {
			if(ls_)
				ls_->set_visible(v);

		    visibility_ = v;
        }

        virtual bool get_visible () override
        {
            return *visibility_;
        }

		void init_()
		{
			if(visibility_)
			 ls_->set_visible(*visibility_);

			if(name_)
		     ls_->set_font(*name_, size_);

			if(labels_man_)
		       ls_->set_labels_manager(labels_man_);
		}

    private:
        label_support_ptr                    ls_;    
        labels_management::labels_manager *  labels_man_;
        optional<bool>                       visibility_;
        optional<std::string>                name_;
        unsigned                             size_;
    };

}