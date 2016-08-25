#pragma once

namespace visual_objects
{
    struct label_support;
    struct parashute_support;
    struct forsage_support;
    struct smoke_support;
    struct landing_dust_support;
    struct morphs_support;

    typedef boost::shared_ptr<label_support>        label_support_ptr;
    typedef boost::shared_ptr<parashute_support>    parashute_support_ptr;
    typedef boost::shared_ptr<forsage_support>      forsage_support_ptr;
    typedef boost::shared_ptr<smoke_support>        smoke_support_ptr;
    typedef boost::shared_ptr<landing_dust_support> landing_dust_support_ptr;
    typedef boost::shared_ptr<morphs_support>       morphs_support_ptr;
}