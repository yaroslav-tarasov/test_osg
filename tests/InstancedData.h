#pragma once

struct image_data
{
    int s;
    int t;
    int r;
    GLenum pixelFormat;
    GLenum type;
    GLint internalFormat;
    size_t                  data_num;
    std::vector<unsigned char>  data;
    std::vector<std::string>   bones;

    image_data ()
    {}

    image_data (const osg::Image* image)
    {
        s = image->s();
        t = image->t();
        r = image->r();
        pixelFormat = image->getPixelFormat(); 
        type  = image->getDataType();
        internalFormat = image->getInternalTextureFormat();
    }
};

REFL_STRUCT(image_data)
    REFL_ENTRY(s)
    REFL_ENTRY(t)
    REFL_ENTRY(r)
    REFL_ENTRY(pixelFormat) 
    REFL_ENTRY(internalFormat) 
    REFL_ENTRY(type)
    REFL_ENTRY(data_num)
    REFL_ENTRY(data)
    REFL_ENTRY(bones)
REFL_END()

namespace avAnimation
{
    typedef std::vector<std::pair<std::string, std::vector<osg::Matrix>>>     AnimationChannelMatricesType;
    typedef std::unordered_map<std::string, AnimationChannelMatricesType> AnimationDataType;
}