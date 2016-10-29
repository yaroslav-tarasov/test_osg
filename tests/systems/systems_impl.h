#pragma once 

namespace first
{
    class impl : public systems,
        public boost::enable_shared_from_this<impl>
    {
    public:
        impl(remote_send_f rs);

        systems_ptr                get_this()  override;

    private:
        void                       push_back_all (binary::bytes_cref bytes);
        void                       push_back (binary::bytes_cref bytes);
        virtual  void              update_messages();
        virtual  void              update_vis_messages();
        virtual kernel::system_ptr get_control_sys()                override;
        virtual kernel::system_ptr get_visual_sys(av::IVisualPtr vis) override;
        virtual kernel::system_ptr get_model_sys()                  override;
        virtual void               create_auto_objects()            override;                                                            


        std::deque<binary::bytes_t>                           queue_;
        std::deque<binary::bytes_t>                       queue_vis_;

        kernel::msg_service                             msg_service_;
        kernel::msg_service                         msg_service_vis_;

        kernel::system_ptr                                     _msys;
        kernel::system_ptr                                     _vsys;
        kernel::system_ptr                                     _csys;

		kernel::systems_factory_ptr							_sys_fab;
    };
}