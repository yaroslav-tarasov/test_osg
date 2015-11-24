#pragma once

namespace avCore
{

    namespace Noise
    {

    osg::Texture2D* generate2DTex(float baseFreq, float persistence, int w, int h, bool periodic);

    }

}