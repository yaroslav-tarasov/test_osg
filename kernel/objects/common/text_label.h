#pragma once 

#include "common/unicode_support.h"
#include "common/labels_management.h"
#include "common/aircraft_support_fwd.h"

namespace visual_objects
{
    struct label_support : labels_management::label_provider

    {
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
             return true;
        }

    private:
        double                               radius_;
        osg::Node *                          label_;

        visual_object_ptr                    visual_object_;
        labels_management::labels_manager *  labels_man_;
    };
    
    struct label_support_proxy : labels_management::label_provider
    {

        label_support_proxy(label_support_ptr ls)
            : ls_ (ls)
        {}

    private:
        void set_labels_manager( labels_management::labels_manager * man  )     override
        {
            labels_man_ = man;
        }

        void set_font( std::string const & name  , unsigned size ) override
        {
            name_ = name;
            size_ = size;
        }

        virtual void set_visible (bool v) override
        {
            visibility_ = v;
        }

        virtual bool get_visible () override
        {
            return visibility_;
        }

    private:
        label_support_ptr                    ls_;    
        labels_management::labels_manager *  labels_man_;
        bool                                 visibility_;
        std::string                          name_;
        unsigned                             size_;
    };

}