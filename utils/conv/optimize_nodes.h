#pragma once

namespace do_not_optimize
{
    const char* names[] =
    {
        "port",
        "starboard",
        "tail",
        "steering_lamp",
        "strobe_",
        "landing_lamp",
        "camera_tower",
        "headlight",
        "masts",
        "lightmast",
        "navaid_",
        "tow_point",
        "rtow_point",
        "back_tail",
        "rotor",
        "parachute"
    };

    struct list_t
    {
        list_t()
        {
            for(int i=0; i<sizeof(do_not_optimize::names)/sizeof(do_not_optimize::names[0]);++i)
            {
                nnames.push_back(do_not_optimize::names[i]);
            }
        }

        static bool find_in ( const std::string& node_name )
        {
            bool ret = false;
            for(int i=0; i<sizeof(do_not_optimize::names)/sizeof(do_not_optimize::names[0]);++i)
            {
                 ret |= boost::starts_with(node_name, do_not_optimize::names[i]);
            }

            return ret;
        }

        std::list<string>   nnames;
    };

}
 