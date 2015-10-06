#include "stdafx.h"

#include "kernel/systems/systems_base.h"
#include "kernel/systems/fake_system.h"
#include "kernel/object_class.h"

#include "kernel/systems.h"
#include "kernel/msg_proxy.h"

#include "test_systems.h"

namespace
{

    void create_auto_object(kernel::system_ptr sys, std::string class_name, std::string unique_name)
    {
        using namespace kernel;

        std::vector<object_class_ptr> const &classes = kernel::fake_objects_factory_ptr(sys)->object_classes() ;

        kernel::object_class_ptr class_ptr ;

        for (auto it = classes.begin(), end = classes.end(); it != end; ++it)
        {
            if (class_name == (*it)->name())
                class_ptr = *it ;
            std::string n = (*it)->name();
        }

        auto obj = kernel::fake_objects_factory_ptr(sys)->create_object(class_ptr, unique_name); 

    }

    FIXME("Не используется")
    kernel::object_info_ptr create_object(kernel::system_ptr sys, std::string class_name, std::string unique_name)
    {
        using namespace kernel;

        std::vector<object_class_ptr> const &classes = kernel::fake_objects_factory_ptr(sys)->object_classes() ;

        kernel::object_class_ptr class_ptr ;

        for (auto it = classes.begin(), end = classes.end(); it != end; ++it)
        {
            if (class_name == (*it)->name())
                class_ptr = *it ;
            std::string n = (*it)->name();
        }

        return kernel::fake_objects_factory_ptr(sys)->create_object(class_ptr, unique_name); 
    }


    class impl : public creator,
                 public boost::enable_shared_from_this<impl>
    {
    public:
        impl()
            : _msys(nullptr)
            , _csys(nullptr)
            , _vsys(nullptr)
        {

        }

        creator_ptr  get_this()
        {
            return shared_from_this();
        }

    private:
        virtual kernel::system_ptr get_control_sys() override 
        { 
            if(!_csys)
                _csys = create_ctrl_system(msg_service_);
            return  _csys;
        };

        virtual kernel::system_ptr get_visual_sys()  override 
        {
            kernel::vis_sys_props props_;
            props_.base_point = ::get_base();
       
            FIXME(damn properties)
            if (!_vsys)
                _vsys = create_visual_system(msg_service_, props_);
            return  _vsys;
        };

        virtual kernel::system_ptr get_model_sys()   override 
        {
            if (!_msys)
                _msys = create_model_system(msg_service_, "place script here");
            return  _msys;
        };

        virtual void create_auto_objects()   override
        {
            create_auto_object(_msys,"phys_sys","phys_sys");
            create_auto_object(_msys,"airports_manager","aiports_manager");
            create_auto_object(_msys,"ada","ada");
            create_auto_object(_msys,"meteo_proxy","meteo_proxy");
        }



        kernel::msg_service                             msg_service_;
        kernel::system_ptr                                     _msys;
        kernel::system_ptr                                     _vsys;
        kernel::system_ptr                                     _csys;

    };

}


creator_ptr sys_creator()
{
    static  shared_ptr<impl> cc = make_shared<impl>();
    return cc->get_this();
}


