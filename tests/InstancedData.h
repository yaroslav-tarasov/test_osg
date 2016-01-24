#pragma once

struct animation_data
{
	std::string name;
	uint32_t    frames;
	uint32_t    seconds;
};

REFL_STRUCT(animation_data)
	REFL_ENTRY(name)
	REFL_ENTRY(frames)
	REFL_ENTRY(seconds)
REFL_END()

struct image_data
{
    int                         s;
    int                         t;
    int                         r;
    GLenum                      pixelFormat;
    GLenum                      type;
    GLint                       internalFormat;
    uint32_t                    data_len;
    std::vector<unsigned char>  data;
    std::vector<std::string>    bones;
    std::vector<image_data>     animations;

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
    REFL_ENTRY(data_len)
    REFL_ENTRY(data)
    REFL_ENTRY(bones)
    REFL_ENTRY(animations)
REFL_END()

namespace avAnimation
{
    typedef std::vector<std::pair<std::string, std::vector<osg::Matrix>>>     AnimationChannelMatricesType;
    typedef std::unordered_map<std::string, AnimationChannelMatricesType>     AnimationDataType;
}