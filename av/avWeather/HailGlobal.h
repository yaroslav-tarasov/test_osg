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
    // Global falling hail class
    //

    class HailGlobal : public PrecipitationBase
    {

    public:

        // constructor
        HailGlobal()
            : PrecipitationBase("HailDrop", 20000, osg::Vec3f(30.f, 30.f, 20.f), 8.f)
        {
            return;
        }

    private:

        // nothing yet
    };
}