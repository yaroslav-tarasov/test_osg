#pragma once

namespace aircraft
{
    enum rotors_group
    {
        RG_FRONT = 0, RG_REAR_LEFT, RG_REAR_RIGHT, RG_REAR_LEFT2, RG_REAR_RIGHT2, RG_REAR_LEFT3, RG_REAR_RIGHT3, RG_TAIL, RG_MAIN, RG_MAIN_1=RG_MAIN, RG_MAIN_2, RG_LAST
    };

    struct rotors_support
    {
        virtual void visit_groups   (std::function<void(rotors_group_t &,size_t&)> out) = 0;
        virtual void visit_rotors   (std::function<void(rotors_group_t const& ,size_t&)> out) = 0;
        virtual void set_malfunction(rotors_group group, bool val) = 0;
        virtual void freeze() = 0;

        virtual ~rotors_support() {}
    };

    typedef polymorph_ptr<rotors_support> rotors_support_ptr;

}