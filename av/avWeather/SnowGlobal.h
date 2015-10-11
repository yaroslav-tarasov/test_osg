#pragma once

//
// Local includes
//

#include <av/avWeather/PrecipitationBase.h>

//
// Local namespaces
//

namespace avWeather
{

    //
    // Global falling snow class
    //

    class SnowGlobal : public PrecipitationBase
    {

    public:

        // constructor
        SnowGlobal()
            : PrecipitationBase("SnowFlake", 35000, osg::Vec3f(25.f, 25.f, 15.f), 8.f)
        {
            return;
        }

    private:

        // nothing yet
    };
}