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
    // Global falling rain class
    //

    class RainGlobal : public PrecipitationBase
    {

    public:

        // constructor
        RainGlobal()
            : PrecipitationBase("RainDrop", 32000, osg::Vec3f(25.f, 25.f, 15.f), 3.f)
        {
            return;
        }

    private:

        // nothing yet
    };
}