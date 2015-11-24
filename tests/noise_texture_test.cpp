#include "stdafx.h"

#include "av/precompiled.h"

#include <glm/glm.hpp>
#include <glm/gtc/noise.hpp>

namespace avCore
{

namespace Noise
{

	template <typename T>
osg::Texture2D* storeTex( T * data, int w, int h ) 
{
    //GLuint texID;
    //glGenTextures(1, &texID);

    //glBindTexture(GL_TEXTURE_2D, texID);
    //glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, w, h);
    //glTexSubImage2D(GL_TEXTURE_2D,0,0,0,w,h,GL_RGBA,GL_UNSIGNED_BYTE,data);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    osg::Image* image = new osg::Image;

    image->setImage(w, h, 1,
        //GL_RGBA, /*GL_LUMINANCE*/GL_RGBA8, GL_UNSIGNED_BYTE,
        GL_RGBA32F_ARB, GL_RGBA, GL_FLOAT,
        reinterpret_cast<unsigned char *>(data),
        osg::Image::USE_NEW_DELETE);
	
	osg::Texture2D* pTexture = new osg::Texture2D;
	//tex->setTextureSize(w, h);
	pTexture->setFilter(osg::Texture2D::MIN_FILTER,osg::Texture2D::LINEAR);
	pTexture->setFilter(osg::Texture2D::MAG_FILTER,osg::Texture2D::LINEAR);
	pTexture->setWrap(osg::Texture::WRAP_S, osg::Texture::REPEAT);
	pTexture->setWrap(osg::Texture::WRAP_T, osg::Texture::REPEAT);

    pTexture->setImage(image);
	
	pTexture->setUseHardwareMipMapGeneration(false);

    return pTexture;
}


osg::Texture2D* generate2DTex(float baseFreq, float persistence, int w, int h, bool periodic) 
{

    int width = w;
    int height = h;

    auto *data = new /*unsigned char*/float[ width * height * 4 ];

    double xFactor = 1.0f / (width - 1);
    double yFactor = 1.0f / (height - 1);

    for( int row = 0; row < height; row++ ) {
        for( int col = 0 ; col < width; col++ ) {
            float x = xFactor * col;
            float y = yFactor * row;
            float sum = 0.0f;
            float freq = baseFreq;
            float persist = persistence;
            for( int oct = 0; oct < 4; oct++ ) {
                glm::vec2 p(x * freq, y * freq);

                float val = 0.0f;
                if (periodic) {
                    val = glm::perlin(p, glm::vec2(freq)) * persist;
                } else {
                    val = glm::perlin(p) * persist;
                }

                sum += val;

                float result = (sum + 1.0f) / 2.0f;

                // Clamp strictly between 0 and 1
                result = result > 1.0f ? 1.0f : result;
                result = result < 0.0f ? 0.0f : result;

                // Store in texture
                data[((row * width + col) * 4) + oct] = /*(unsigned char)*/ ( result * 255.0f );
                freq *= 2.0f;
                persist *= persistence;
            }
        }
    }


    osg::Texture2D* tex = storeTex(data, width, height);
    //delete [] data;

    return tex;
}

}

}
//AUTO_REG(noise_texture_test)