#include "stdafx.h"
#include "find_node_visitor.h" 
#include "creators.h"
#include <windows.h>

// #define TEST_SHADOWS
// #define TEST_TEXTURE
// #define TEST_ADLER_SCENE

#define TEXUNIT_SINE         1
#define TEXUNIT_NOISE        2


namespace
{
    const osg::Vec4 red_color   = osg::Vec4(100.0f, 0.0f, 0.0f, 100.0f);
    const osg::Vec4 blue_color  = osg::Vec4(0.0f, 0.0f, 100.0f, 100.0f);
    const osg::Vec4 green_color = osg::Vec4(0.0f, 100.0f, 0.0f, 100.0f);
    const osg::Vec4 white_color = osg::Vec4(100.0f, 100.0f, 100.0f, 100.0f);
    const osg::Vec4 black_color = osg::Vec4(0.0f,0.0f,0.0f,100.0f);
    const osg::Vec4 gray_color  = osg::Vec4(0.8f,0.8f,0.8f,100.0f);


    char vertexShaderSource_simple[] =  STRINGIFY ( 
        uniform vec4 coeff;
        void main(void)
        {
            gl_TexCoord[0] = gl_Vertex; 
            vec4 vert = gl_Vertex;
            vert.z = gl_Vertex.x*coeff[0] + gl_Vertex.x*gl_Vertex.x* coeff[1] + 
                gl_Vertex.y*coeff[2] + gl_Vertex.y*gl_Vertex.y* coeff[3];
            gl_Position = gl_ModelViewProjectionMatrix * vert;
        }
    
    );

    const char vs[] = STRINGIFY ( 
        // #pragma debug(on)
        //uniform vec3 LightPosition;
        uniform vec4 coeff;
        // const vec3 LightPosition = vec3(0.0, 0.0, 4.0);

        const float specularContribution = 0.3;
        const float diffuseContribution  = (1.0 - specularContribution);

        /*varying*/out float LightIntensity;
        /*varying*/out vec2  MCposition;

        void main(void)
        {
            vec3 LightPosition = vec3(100.0, 100.0, 400.0) * coeff.xyz;
            vec4 ecPosition = gl_ModelViewMatrix * gl_Vertex;
            vec3 tnorm      = normalize(gl_NormalMatrix * gl_Normal);
            vec3 lightVec   = normalize(LightPosition - vec3 (ecPosition));
            vec3 reflectVec = reflect(-lightVec, tnorm);
            vec3 viewVec    = normalize(vec3 (-ecPosition));
            float spec      = max(dot(reflectVec, viewVec), 0.0);
            spec            = pow(spec, 16.0);
            LightIntensity  = diffuseContribution * max(dot(lightVec, tnorm), 0.0) +
                specularContribution * spec;
            MCposition      = gl_Vertex.xz;
            gl_Position     = gl_ModelViewProjectionMatrix * gl_Vertex;
        }
    );

    const char fs[] = STRINGIFY ( 
            // uniform vec3  BrickColor, MortarColor;
            // uniform float ColumnWidth, RowHeight;
            // uniform float Bwf, Bhf;

            const vec3 BrickColor = vec3(1.0, 0.3, 0.2);
            const vec3 MortarColor = vec3(0.85, 0.85, 0.85);
            const float ColumnWidth = 0.30;
            const float RowHeight = 0.15;
            const float Bwf = 0.95;
            const float Bhf = 0.90;

            /*varying*/in vec2  MCposition;
            /*varying*/in float LightIntensity;

            void main(void)
            {
                vec3 color;
                float ss;
                float tt;
                float w;
                float h;

                ss = MCposition.x / ColumnWidth;
                tt = MCposition.y / RowHeight;

                if (fract(tt * 0.5) > 0.5)
                    ss += 0.5;

                ss = fract(ss);
                tt = fract(tt);

                w = step(ss, Bwf);
                h = step(tt, Bhf);

                color = mix(MortarColor, BrickColor, w * h) * LightIntensity;
                gl_FragColor = vec4 (color, 1.0);
            }
    );
    
    namespace marible
    {
        const char vs[] = {"#version 110 \n"  
            STRINGIFY ( 
                //layout (location = 0) out float LightIntensity ;
                //layout (location = 1) out vec3  MCposition ;
                varying float LightIntensity ;
                varying vec3  MCposition ;
        
                const float Scale = 1.0;
                const vec3  LightPos = vec3( 0.0, 0.0, 4.0 ) ;

                //uniform mat4 gl_NormalMatrix;
                //uniform mat4 gl_ModelViewProjectionMatrix;
                //uniform mat4 gl_ModelViewMatrix;
                //uniform vec4 gl_Vertex;
                //uniform vec4 gl_Normal;

                void main(void)
                {
                    vec4 ECposition = gl_ModelViewMatrix * gl_Vertex;
                    MCposition      = vec3 (gl_Vertex) * Scale;
                    vec3 tnorm      = normalize(vec3 (gl_NormalMatrix * gl_Normal));
                    LightIntensity  = dot(normalize(LightPos - vec3 (ECposition)), tnorm) * 1.5;
                    gl_Position     = gl_ModelViewProjectionMatrix * gl_Vertex;
                }
            )
        };

    const char fs[] = { "#version 110 \n"
        STRINGIFY ( 

            //layout (location = 0) in float LightIntensity; 
            //layout (location = 1) in vec3  MCposition;
            varying float LightIntensity ;
            varying vec3  MCposition ;
        
            const vec3 MarbleColor = vec3( 0.7, 0.7, 0.7 );
            const vec3 VeinColor = vec3( 0.0, 0.15, 0.0 );

            uniform sampler3D NoiseTex;
            uniform sampler1D SineTex;
            uniform vec3 Offset;

            //out vec4 gl_FragColor;

            void main (void)
            {
                vec4 noisevec   = texture3D(NoiseTex, MCposition + Offset.yzx);// texture(NoiseTex, MCposition + Offset.yzx);

                float intensity = abs(noisevec[0] - 0.25) +
                    abs(noisevec[1] - 0.125) +
                    abs(noisevec[2] - 0.0625) +
                    abs(noisevec[3] - 0.03125);

                vec4 unswiz = texture1D(SineTex, MCposition.z + intensity * 2.0);//texture(SineTex, MCposition.z + intensity * 2.0);
                float sineval = unswiz.s;
                vec3 color   = mix(VeinColor, MarbleColor, sineval);
                color       *= LightIntensity;
                color = clamp(color, 0.0, 1.0);
                gl_FragColor = vec4 (color, 1.0);
            }
        )
    };

    }

    namespace circles
    {
        static const char vs[] = STRINGIFY ( 

            void main(void)
            {
                // gl_Position = gl_Vertex;
                gl_Position     = ftransform();//gl_ModelViewProjectionMatrix *  gl_Vertex; 
            }
        );

        static const char fs[] = STRINGIFY ( 
         void main(void)
         {
                vec2 pos = mod(gl_FragCoord.xy, vec2(50.0)) - vec2(25.0);
                float dist_squared = dot(pos, pos);
            
                gl_FragColor = (dist_squared < 400.0) 
                    ? vec4(.90, .90, .90, 1.0)
                    : vec4(.20, .20, .40, 1.0);
         }
        );

    }
}

namespace effects 
{ 
    
    osg::Vec3 wind(10.0f,10.0f,0.0f); 

    void insertParticle(osg::Group* root,osg::Node* rootModel, const osg::Vec3& center, float radius)
    {
        bool handleMovingModels = false;

        osg::Vec3 position = center + 
            osg::Vec3( radius * (((float)rand() / (float)RAND_MAX)-0.5)*2.0,
            radius * (((float)rand() / (float)RAND_MAX)-0.5)*2.0,
            0.0f);
   

        float scale = 50.0f * ((float)rand() / (float)RAND_MAX);
        float intensity = 10.0f;

        osgParticle::ExplosionEffect* explosion = new osgParticle::ExplosionEffect(position, scale, intensity);
        osgParticle::ExplosionDebrisEffect* explosionDebri = new osgParticle::ExplosionDebrisEffect(position, scale, intensity);
        osgParticle::FireEffect* fire = new osgParticle::FireEffect(position, scale, intensity);
        osgParticle::ParticleEffect* smoke = 0;
        //if (handleMovingModels)
            smoke =  new osgParticle::SmokeTrailEffect(position, scale, intensity);
        //else
        //    smoke =  new osgParticle::SmokeEffect(position, scale, intensity);
       
       // Если раскомментить форма дыма поменяется но это будет жеееееесть
       // smoke->setTextureFileName("Images/continous_black_smoke.rgb");
        
	    explosion->setWind(wind);
        explosionDebri->setWind(wind);
        smoke->setWind(wind);
        fire->setWind(wind);

        osg::Group* effectsGroup = new osg::Group;
        effectsGroup->addChild(explosion);
        effectsGroup->addChild(explosionDebri);
        effectsGroup->addChild(smoke);
        effectsGroup->addChild(fire);
        
        
        explosion->setUseLocalParticleSystem(false);
        explosionDebri->setUseLocalParticleSystem(false);
        smoke->setUseLocalParticleSystem(false);
        fire->setUseLocalParticleSystem(false);
       
        osg::ref_ptr<osg::Node> hitNode = rootModel;// hit.nodePath.back();
        osg::Node::ParentList parents = hitNode->getParents();                
        osg::Group* insertGroup = 0;
        unsigned int numGroupsFound = 0;
        for(osg::Node::ParentList::iterator itr=parents.begin();
            itr!=parents.end();
            ++itr)
        {
            if (typeid(*(*itr))==typeid(osg::Group))
            {
                ++numGroupsFound;
                insertGroup = *itr;
            }
        }         

        if (numGroupsFound==parents.size() && numGroupsFound==1 && insertGroup)
        {
            osg::notify(osg::INFO)<<"PickHandler::pick(,) hit node's parent is a single osg::Group so we can simple the insert the particle effects group here."<<std::endl;

            // just reuse the existing group.
            insertGroup->addChild(effectsGroup);
        }
        else
        {            
            osg::notify(osg::INFO)<<"PickHandler::pick(,) hit node doesn't have an appropriate osg::Group node to insert particle effects into, inserting a new osg::Group."<<std::endl;
            insertGroup = new osg::Group;
            for(osg::Node::ParentList::iterator itr=parents.begin();
                itr!=parents.end();
                ++itr)
            {
                (*itr)->replaceChild(rootModel/*hit.nodePath.back()*/,insertGroup);
            }
            insertGroup->addChild(hitNode.get());
            insertGroup->addChild(effectsGroup);
        }

        // finally insert the particle systems into a Geode and attach to the root of the scene graph so the particle system
        // can be rendered.
        osg::Geode* geode = new osg::Geode;
        geode->addDrawable(explosion->getParticleSystem());
        geode->addDrawable(explosionDebri->getParticleSystem());
        geode->addDrawable(smoke->getParticleSystem());
        geode->addDrawable(fire->getParticleSystem());
        
        root->addChild(geode);
        
        // rootModel->addChild(effectsGroup);
    }

    osg::Node* createLightSource( unsigned int num,
        const osg::Vec3& trans,
        const osg::Vec4& color )
    {
        auto CreateLight = [=](const osg::Vec4& fcolor)->osg::Geode* {
            osg::ref_ptr<osg::ShapeDrawable> shape1 = new osg::ShapeDrawable();
            shape1->setShape( new osg::Sphere(osg::Vec3(0.0f, 0.0f, 0.0f), 2.f) );
            shape1->setColor( fcolor );
            osg::Geode* light = new osg::Geode;
            light->addDrawable( shape1.get() );
            return light;
        };

        osg::ref_ptr<osg::Geode> light_src   = CreateLight(osg::Vec4(255.0f,0.0f,0.0f,255.0f));

        osg::ref_ptr<osg::Light> light = new osg::Light;
        light->setLightNum( num );
        light->setDiffuse( color );
        //light->setDiffuse(osg::Vec4(1.0,1.0,1.0,1.0));
        //light->setSpecular(osg::Vec4(1,1,1,1));  // some examples don't have this one

        light->setPosition( osg::Vec4(0.0f, 0.0f, 0.0f, 1.0f) );
        osg::ref_ptr<osg::LightSource> lightSource = new   osg::LightSource;
        lightSource->setLight( light );

        osg::ref_ptr<osg::MatrixTransform> sourceTrans =
            new osg::MatrixTransform;
        sourceTrans->setMatrix( osg::Matrix::translate(trans) );
        sourceTrans->addChild( lightSource.get() );
        sourceTrans->addChild( light_src.get() );
        return sourceTrans.release();
    }


    class BlinkNode: public osg::NodeCallback
    {
    
        float     _previous;
        short     _curr_interval;
        osg::Vec4 _first_color;
        osg::Vec4 _second_color;
    public:
        BlinkNode(osg::Vec4 first_color,osg::Vec4 second_color)
        : _previous (0.0f)
        , _curr_interval(0)
        , _first_color(first_color/*osg::Vec4(255.0f, 255.0f, 255.0f, 100.0f)*/)
        , _second_color(second_color/*osg::Vec4(0.0f, 0.0f, 0.0f, 100.0f)*/)
        {

        }

        void operator()(osg::Node* node, osg::NodeVisitor* nv) {
            if (!dynamic_cast<osg::Geode *>(node)) return;
            const osg::Vec4 f_color[] = {_first_color,_second_color,_first_color,_second_color};
            const float     _intervals[] = {0.5f, 0.5f,.5f,2.f};

            static_assert(sizeof(_intervals)/sizeof(_intervals[0]) == sizeof(f_color)/sizeof(f_color[0]),"Size of time intervals array and colors array must be same");

            double t = nv->getFrameStamp()->getSimulationTime();

            if(_previous == 0.0f) _previous = t;

            // _motion->update(t - _previous);
            if (t - _previous > _intervals[_curr_interval])
            {
                _previous = t;
                _curr_interval =  ++_curr_interval % static_cast<short>(sizeof(_intervals)/sizeof(_intervals[0]));
                dynamic_cast<osg::ShapeDrawable *>(node->asGeode()->getDrawable(0))->setColor( f_color[_curr_interval] );
            }

        }

    };
    
    osg::Image*
        make1DSineImage( int texSize )
    {
        const float PI = 3.1415927;

        osg::Image* image = new osg::Image;
        image->setImage(texSize, 1, 1,
            4, GL_RGBA, GL_UNSIGNED_BYTE,
            new unsigned char[4 * texSize],
            osg::Image::USE_NEW_DELETE);

        GLubyte* ptr = image->data();
        float inc = 2. * PI / (float)texSize;
        for(int i = 0; i < texSize; i++)
        {
            *ptr++ = (GLubyte)((sinf(i * inc) * 0.5 + 0.5) * 255.);
            *ptr++ = 0;
            *ptr++ = 0;
            *ptr++ = 1;
        }
        return image;        
    }

    osg::Texture1D*
        make1DSineTexture( int texSize )
    {
        osg::Texture1D* sineTexture = new osg::Texture1D;
        sineTexture->setWrap(osg::Texture1D::WRAP_S, osg::Texture1D::REPEAT);
        sineTexture->setFilter(osg::Texture1D::MIN_FILTER, osg::Texture1D::LINEAR);
        sineTexture->setFilter(osg::Texture1D::MAG_FILTER, osg::Texture1D::LINEAR);
        sineTexture->setImage( make1DSineImage(texSize) );
        return sineTexture;
    }

    class UniformVarying : public osg::Uniform::Callback
    {
        virtual void operator () (osg::Uniform* uniform, osg::NodeVisitor* nv)
        {
            const osg::FrameStamp* fs = nv->getFrameStamp();
            float value = sinf(fs->getSimulationTime());
            std::string  name = uniform->getName();
            uniform->set(osg::Vec4(value,-value,-value,value));
        }
    };

    class AnimateCallback: public osg::Uniform::Callback
    {
    public:
        virtual void operator() ( osg::Uniform* uniform, osg::NodeVisitor* nv )
        {
                float angle = 2.0 * nv->getFrameStamp()->getSimulationTime();
                float sine = sinf( angle );        // -1 -> 1
                float v01 = 0.5f * sine + 0.5f;        //  0 -> 1
                float v10 = 1.0f - v01;                //  1 -> 0

                uniform->set( osg::Vec3(0.505f, 0.8f*v01, 0.0f) ); 
        }
    };

    template<typename G>
    void createShader(G* geom)
    {
        osg::StateSet* stateset = geom->getOrCreateStateSet();
        
        osg::Texture3D* noiseTexture = osgUtil::create3DNoiseTexture( 32 /*128*/ );
        osg::Texture1D* sineTexture = make1DSineTexture( 32 /*1024*/ );

        stateset->setTextureAttribute(TEXUNIT_NOISE, noiseTexture);
        stateset->setTextureAttribute(TEXUNIT_SINE, sineTexture);

        osg::Program* program = new osg::Program;


        stateset->setAttribute(program);

        auto vs = new osg::Shader(osg::Shader::VERTEX, marible::vs);
        auto fs = new osg::Shader( osg::Shader::FRAGMENT, marible::fs );
        program->addShader( vs );
        program->addShader( fs );
        
        // FIXME Здесь получаем ошибки и прерываение выполнения шейдеров
        //std::string log;
        //program->getGlProgramInfoLog(0,log);
        


        //osg::Uniform* coeff = new osg::Uniform("coeff",osg::Vec4(1.0,-1.0f,-1.0f,1.0f));
        //stateset->addUniform(coeff);
        
        stateset->addUniform( new osg::Uniform("NoiseTex", TEXUNIT_NOISE) );
        stateset->addUniform( new osg::Uniform("SineTex", TEXUNIT_SINE) ); 
        osg::Uniform* uni_offset = new osg::Uniform("Offset", osg::Vec3(0.0f, 0.0f, 0.0f));
        stateset->addUniform( uni_offset );

        if (true)
        {
            //coeff->setUpdateCallback(new UniformVarying);
            //coeff->setDataVariance(osg::Object::DYNAMIC);
            uni_offset->setUpdateCallback(new AnimateCallback);
            uni_offset->setDataVariance(osg::Object::DYNAMIC);
            stateset->setDataVariance(osg::Object::DYNAMIC);
        }

        findNodeVisitor::nodeNamesList list_name;
        list_name.push_back("white_blink");
        list_name.push_back("red");
        list_name.push_back("blue");
        list_name.push_back("green");

        findNodeVisitor findNodes(list_name); 
        geom->accept(findNodes);

        findNodeVisitor::nodeListType& wln_list = findNodes.getNodeList();

        for(auto it = wln_list.begin(); it != wln_list.end(); ++it )
        {
            (*it)->getOrCreateStateSet()->setAttribute(new osg::Program(), osg::StateAttribute::OVERRIDE | osg::StateAttribute::PROTECTED | osg::StateAttribute::OFF);
        }

    }


}

namespace lights
{
    osg::Image* createSpotLightImage(const osg::Vec4& centerColour, const osg::Vec4& backgroudColour, unsigned int size, float power)
    {
        osg::Image* image = new osg::Image;
        image->allocateImage(size,size,1,
            GL_RGBA,GL_UNSIGNED_BYTE);


        float mid = (float(size)-1)*0.5f;
        float div = 2.0f/float(size);
        for(unsigned int r=0;r<size;++r)
        {
            unsigned char* ptr = image->data(0,r,0);
            for(unsigned int c=0;c<size;++c)
            {
                float dx = (float(c) - mid)*div;
                float dy = (float(r) - mid)*div;
                float r = powf(1.0f-sqrtf(dx*dx+dy*dy),power);
                if (r<0.0f) r=0.0f;
                osg::Vec4 color = centerColour*r+backgroudColour*(1.0f-r);
                *ptr++ = (unsigned char)((color[0])*255.0f);
                *ptr++ = (unsigned char)((color[1])*0.0f);
                *ptr++ = (unsigned char)((color[2])*0.0f);
                *ptr++ = (unsigned char)((color[3])*255.0f);
            }
        }
        return image;

        //return osgDB::readImageFile("spot.dds");
    }

    osg::StateSet* createSpotLightDecoratorState(unsigned int lightNum, unsigned int textureUnit)
    {
        osg::StateSet* stateset = new osg::StateSet;

        stateset->setMode(GL_LIGHT0+lightNum, osg::StateAttribute::ON);

        osg::Vec4 centerColour(1.0f,0.0f,0.0f,1.0f);
        osg::Vec4 ambientColour(0.05f,0.f,0.f,1.0f); 

        // set up spot light texture
        osg::Texture2D* texture = new osg::Texture2D();
        texture->setImage(createSpotLightImage(centerColour, ambientColour, 64, 1.0));
        texture->setBorderColor(osg::Vec4(ambientColour));
        texture->setWrap(osg::Texture::WRAP_S,osg::Texture::CLAMP_TO_BORDER);
        texture->setWrap(osg::Texture::WRAP_T,osg::Texture::CLAMP_TO_BORDER);
        texture->setWrap(osg::Texture::WRAP_R,osg::Texture::CLAMP_TO_BORDER);

        stateset->setTextureAttributeAndModes(textureUnit, texture, osg::StateAttribute::ON);

        // set up tex gens
        stateset->setTextureMode(textureUnit, GL_TEXTURE_GEN_S, osg::StateAttribute::ON);
        stateset->setTextureMode(textureUnit, GL_TEXTURE_GEN_T, osg::StateAttribute::ON);
        stateset->setTextureMode(textureUnit, GL_TEXTURE_GEN_R, osg::StateAttribute::ON);
        stateset->setTextureMode(textureUnit, GL_TEXTURE_GEN_Q, osg::StateAttribute::ON);

        return stateset;
    }


    osg::Node* createSpotLightNode(const osg::Vec3& position, const osg::Vec3& direction, float angle, unsigned int lightNum, unsigned int textureUnit)
    {
        osg::Group* group = new osg::Group;

        // create light source.
        osg::LightSource* lightsource = new osg::LightSource;
        osg::Light* light = lightsource->getLight();
        light->setLightNum(lightNum);
        light->setPosition(osg::Vec4(position,1.0f));
        light->setAmbient(osg::Vec4(0.00f,0.00f,0.05f,1.0f));
        light->setDiffuse(osg::Vec4(1.0f,1.0f,1.0f,1.0f));
        group->addChild(lightsource);

        // create tex gen.

        osg::Vec3 up(0.0f,0.0f,1.0f);
        up = (direction ^ up) ^ direction;
        up.normalize();

        osg::TexGenNode* texgenNode = new osg::TexGenNode;
        texgenNode->setTextureUnit(textureUnit);
        osg::TexGen* texgen = texgenNode->getTexGen();
        texgen->setMode(osg::TexGen::EYE_LINEAR);
        texgen->setPlanesFromMatrix(osg::Matrixd::lookAt(position, position+direction, up)*
            osg::Matrixd::perspective(angle,1.0,0.1,100)*
            osg::Matrixd::translate(1.0,1.0,1.0)*
            osg::Matrixd::scale(0.5,0.5,0.5));


        group->addChild(texgenNode);

        return group;

    }
}

namespace creators 
{
    


osg::AnimationPath* createAnimationPath(const osg::Vec3& center,float radius,double looptime)
{
    // set up the animation path
    osg::AnimationPath* animationPath = new osg::AnimationPath;
    animationPath->setLoopMode(osg::AnimationPath::LOOP);

    int numSamples = 400;
    float yaw = 0.0f;
    float yaw_delta = 2.0f*osg::PI/((float)numSamples-1.0f);
    float roll = osg::inDegrees(30.0f);

    double time=0.0f;
    double time_delta = looptime/(double)numSamples;
    for(int i=0;i<numSamples;++i)
    {

#ifdef  YAW_Y_AXE
        osg::Vec3 position(center+osg::Vec3(sinf(yaw)*radius,0.0f,cosf(yaw)*radius));
        osg::Quat rotation(osg::Quat(roll,osg::Vec3(0.0,0.0,1.0))*osg::Quat(/*-*/(yaw+osg::inDegrees(90.0f)),osg::Vec3(0.0,/*-*/1.0,0.0)));
#else
        osg::Vec3 position(center+osg::Vec3(sinf(yaw)*radius,cosf(yaw)*radius,0.0f));
        osg::Quat rotation(osg::Quat(roll,osg::Vec3(0.0,1.0,0.0))*osg::Quat(-(yaw+osg::inDegrees(90.0f)),osg::Vec3(0.0,0.0,1.0)));
#endif
        animationPath->insert(time,osg::AnimationPath::ControlPoint(position,rotation));

        yaw += yaw_delta;
        time += time_delta;

    }
    return animationPath;
}

osg::Node* createBase(const osg::Vec3& center,float radius)
{

    int numTilesX = 10;
    int numTilesY = 10;

    float width = 2*radius;
    float height = 2*radius;

    osg::Vec3 v000(center - osg::Vec3(width*0.5f,height*0.5f,0.0f));
    osg::Vec3 dx(osg::Vec3(width/((float)numTilesX),0.0,0.0f));
    osg::Vec3 dy(osg::Vec3(0.0f,height/((float)numTilesY),0.0f));

    // fill in vertices for grid, note numTilesX+1 * numTilesY+1...
    osg::Vec3Array* coords = new osg::Vec3Array;
    int iy;
    for(iy=0;iy<=numTilesY;++iy)
    {
        for(int ix=0;ix<=numTilesX;++ix)
        {
            coords->push_back(v000+dx*(float)ix+dy*(float)iy);
        }
    }

    //Just two colours - black and white.
    osg::Vec4Array* colors = new osg::Vec4Array;
    colors->push_back(osg::Vec4(1.0f,1.0f,1.0f,1.0f)); // white
    colors->push_back(osg::Vec4(0.0f,0.0f,0.0f,1.0f)); // black

    osg::ref_ptr<osg::DrawElementsUShort> whitePrimitives = new osg::DrawElementsUShort(GL_QUADS);
    osg::ref_ptr<osg::DrawElementsUShort> blackPrimitives = new osg::DrawElementsUShort(GL_QUADS);

    int numIndicesPerRow=numTilesX+1;
    for(iy=0;iy<numTilesY;++iy)
    {
        for(int ix=0;ix<numTilesX;++ix)
        {
            osg::DrawElementsUShort* primitives = ((iy+ix)%2==0) ? whitePrimitives.get() : blackPrimitives.get();
            primitives->push_back(ix    +(iy+1)*numIndicesPerRow);
            primitives->push_back(ix    +iy*numIndicesPerRow);
            primitives->push_back((ix+1)+iy*numIndicesPerRow);
            primitives->push_back((ix+1)+(iy+1)*numIndicesPerRow);
        }
    }

    // set up a single normal
    osg::Vec3Array* normals = new osg::Vec3Array;
    normals->push_back(osg::Vec3(0.0f,0.0f,1.0f));

    osg::Geometry* geom = new osg::Geometry;
    geom->setVertexArray(coords);

    geom->setColorArray(colors, osg::Array::BIND_PER_PRIMITIVE_SET);

    geom->setNormalArray(normals, osg::Array::BIND_OVERALL);

    geom->addPrimitiveSet(whitePrimitives.get());
    geom->addPrimitiveSet(blackPrimitives.get());

    osg::Geode* geode = new osg::Geode;
    geode->addDrawable(geom);
	//
	//osg::MatrixTransform* positioned = new osg::MatrixTransform;
	//positioned->addChild(geode);
    // 

    return geode;
}

nodes_array_t loadAirplaneParts()
{

    osg::Node* airplane_file = osgDB::readNodeFile("a_319.part.dae"); // a_319.open.dae "a_319.part.dae"  "an_124.dae"

    osg::Node* engine = nullptr; 
    osg::Node* engine_geode = nullptr; 
    osg::Node* lod0 = nullptr; 
    osg::Node* lod3 = nullptr; 

#ifdef ANIMATION_TEST
    if(airplane_file)
    {
        findNodeVisitor findNode("animgroup_shassi_r_r_lod0"); 
        airplane_file->accept(findNode);

        auto anim =  findNode.getFirst();

        auto manager_ =  dynamic_cast<osgAnimation::BasicAnimationManager*> ( anim->getUpdateCallback() );

        if ( manager_ )
        {   
            const osgAnimation::AnimationList& animations =
                manager_->getAnimationList();

            std::cout << "**** Animations ****" << std::endl;

            for ( unsigned int i=0; i<animations.size(); ++i )
            {
                const std::string& name = animations[i]-> getName();
                std::cout << "Animation name: " << name << std::endl;
            }

            std::cout << "********************" << std::endl;
        }
    }
#endif

    airplane_file->setName("all_nodes");

    if(airplane_file)
	{
        auto CreateLight = [=](const osg::Vec4& fcolor,const std::string& name,osg::NodeCallback* callback)->osg::Geode* {
            osg::ref_ptr<osg::ShapeDrawable> shape1 = new osg::ShapeDrawable();
            shape1->setShape( new osg::Sphere(osg::Vec3(0.0f, 0.0f, 0.0f), 0.2f) );
            // shape1->setColor( fcolor );
            osg::Geode* light = new osg::Geode;
            light->addDrawable( shape1.get() );
            dynamic_cast<osg::ShapeDrawable *>(light->getDrawable(0))->setColor( fcolor );
            light->setUpdateCallback(callback);
            light->setName(name);
            const osg::StateAttribute::GLModeValue value = osg::StateAttribute::PROTECTED| osg::StateAttribute::OVERRIDE | osg::StateAttribute::OFF;
            light->getOrCreateStateSet()->setAttribute(new osg::Program(),value);
            light->getOrCreateStateSet()->setTextureAttributeAndModes( 0, new osg::Texture2D(), value );
            light->getOrCreateStateSet()->setTextureAttributeAndModes( 1, new osg::Texture2D(), value );
            return light;
        };

        osg::ref_ptr<osg::Geode> red_light   = CreateLight(red_color,std::string("red"),nullptr);
        osg::ref_ptr<osg::Geode> blue_light  = CreateLight(blue_color,std::string("blue"),nullptr);
        osg::ref_ptr<osg::Geode> green_light = CreateLight(green_color,std::string("green"),nullptr);
        osg::ref_ptr<osg::Geode> white_light = CreateLight(white_color,std::string("white_blink"),new effects::BlinkNode(white_color,gray_color));
		
        auto addAsChild = [=](std::string root,osg::Node* child)->osg::Node* {
            findNodeVisitor findTail(root.c_str()); 
            airplane_file->accept(findTail);
            auto tail =  findTail.getFirst();
            if(tail)  tail->asGroup()->addChild(child);
            return tail;
         };

        auto tail = addAsChild("tail",white_light);
        auto strobe_r = addAsChild("strobe_r",white_light);
        auto strobe_l = addAsChild("strobe_l",white_light);

        auto port = addAsChild("port",green_light);
        auto star_board = addAsChild("starboard",red_light);

        //osg::Node* light0 = effects::createLightSource(
        //    0, osg::Vec3(-20.0f,0.0f,0.0f), red_color);

        //airplane_file->getOrCreateStateSet()->setMode( GL_LIGHT0,
        //    osg::StateAttribute::ON );

        //airplane_file->asGroup()->addChild( light0 );

        
        findNodeVisitor findLod3("Lod3"); 
		airplane_file->accept(findLod3);
		lod3 =  findLod3.getFirst();
        
        if(lod3) 
            lod3->setNodeMask(/*0xffffffff*/0); // Убираем нафиг Lod3 

        findNodeVisitor findLod0("Lod0"); 
        airplane_file->accept(findLod0);
        lod0 =  findLod0.getFirst();


        findNodeVisitor findEngine("engine",findNodeVisitor::not_exact); 
        airplane_file->accept(findEngine);

        engine =  findEngine.getFirst();
        if (engine) engine_geode = engine->asGroup()->getChild(0);//->asGroup()->getChild(0);
	}

    nodes_array_t retval = {nullptr,airplane_file,engine,engine_geode,lod0,lod3};
    return retval;
}


osg::Node* loadAirplane()
{
    return loadAirplaneParts()[1];
}

nodes_array_t createMovingModel(const osg::Vec3& center, float radius)
{
    float animationLength = 50.0f;
    float model_size = 100.0f;

    static std::string texs[] = {"a_319_kuban.png","a_319_airfrance.png","./a_319_aeroflot.png"};

    osg::AnimationPath* animationPath = createAnimationPath(center,radius,animationLength);

    osg::Group* model = new osg::Group;

    osg::Node* glider = osgDB::readNodeFile("t72-tank_des.flt");// glider.osgt

    if (glider)
    {
        const osg::BoundingSphere& bs = glider->getBound();

        float size = model_size/bs.radius()*0.3f;
        osg::MatrixTransform* positioned = new osg::MatrixTransform;
        positioned->setDataVariance(osg::Object::STATIC);
        positioned->setMatrix(osg::Matrix::translate(-bs.center())*
            osg::Matrix::scale(size,size,size)*
            osg::Matrix::rotate(osg::inDegrees(-90.0f),osg::inDegrees(-90.0f),osg::inDegrees(-90.0f),1.0f));
        
       
        positioned->addChild(glider);

        osg::PositionAttitudeTransform* xform = new osg::PositionAttitudeTransform;
        xform->setUpdateCallback(new osg::AnimationPathCallback(animationPath,0.0,1.0));
        xform->addChild(positioned);

#ifdef SPOT_LIGHT        
        //xform->addChild(lights::createSpotLightNode(osg::Vec3(0.0f,0.0f,0.0f), osg::Vec3(0.0f,1.0f,-1.0f), 60.0f, 0, 1));
#endif
        model->addChild(xform);
    }
	
	nodes_array_t plane = loadAirplaneParts();
	auto airplane = plane[1];

    if (airplane)
    {
        airplane->setName("airplane");

#ifdef  TEST_TEXTURE   // Texture on board
        osg::Image *img_plane = osgDB::readImageFile(texs[2]);
        osg::Texture2D *tex_plane = new osg::Texture2D;
        tex_plane->setDataVariance(osg::Object::DYNAMIC);
        
        tex_plane ->setImage(img_plane);
        //airplane->getOrCreateStateSet()->setTextureAttributeAndModes(0, tex_plane ,osg::StateAttribute::ON);
        // Create a new StateSet with default settings: 
        osg::StateSet* stateOne = new osg::StateSet();

        // Assign texture unit 0 of our new StateSet to the texture 
        // we just created and enable the texture.
        stateOne->setTextureAttributeAndModes
            (0,tex_plane,osg::StateAttribute::ON);
        // Associate this state set with the Geode that contains
        // the pyramid: 
        airplane->setStateSet(stateOne);
#endif

        const osg::BoundingSphere& bs = airplane->getBound();
#ifdef  YAW_Y_AXE
        const osg::Quat quat0(osg::inDegrees(0.f/*-90.0f*/), osg::X_AXIS,                      
                              osg::inDegrees(0.f)  , osg::Y_AXIS,
                              osg::inDegrees(0.f)  , osg::Z_AXIS ); 
#else
        const osg::Quat quat0(osg::inDegrees(-90.0f), osg::X_AXIS,                      
                              osg::inDegrees(0.f)  , osg::Y_AXIS,
                              osg::inDegrees(0.f)  , osg::Z_AXIS ); 

#endif
        float size = model_size/bs.radius()*0.7f;
        osg::MatrixTransform* positioned = new osg::MatrixTransform;
        positioned->setDataVariance(osg::Object::DYNAMIC);
        positioned->setMatrix(osg::Matrix::translate(-bs.center())*
            //osg::Matrix::scale(size,size,size)*
            osg::Matrix::rotate(quat0));

        positioned->addChild(airplane);
        
        osg::MatrixTransform* xform = new osg::MatrixTransform;
        xform->setUpdateCallback(new osg::AnimationPathCallback(animationPath,0.0f,2.0));
        xform->addChild(positioned);

        model->addChild(xform);

    }



    nodes_array_t retval = {model,airplane,/*engine*/plane[2],/*engine_geode*/plane[3],/*lod0*/plane[4],/*lod3*/plane[5]};
    return retval;
}

nodes_array_t createModel(bool overlay, osgSim::OverlayNode::OverlayTechnique technique)
{
    osg::Vec3 center(0.0f,0.0f,300.0f);
    float radius = 600.0f;

    osg::Group* root = new osg::Group;

    float baseHeight = 0.0f; //center.z();//-radius*0.5;
#ifdef TEST_ADLER_SCENE 
    const osg::Quat quat0(osg::inDegrees(-90.0f), osg::X_AXIS,                      
        osg::inDegrees(0.f)  , osg::Y_AXIS,
        osg::inDegrees(0.f)  , osg::Z_AXIS ); 

    osg::Node* adler = osgDB::readNodeFile("adler.osgb");
    osg::MatrixTransform* baseModel = new osg::MatrixTransform;
    baseModel->setMatrix(
        // osg::Matrix::translate(-bs.center())*  
        // osg::Matrix::scale(size,size,size)*
        osg::Matrix::rotate(quat0));

    baseModel->addChild(adler);
#else
    osg::Node* baseModel = createBase(osg::Vec3(center.x(), center.y(), baseHeight),radius*3);
#endif
    
    auto ret_array  = createMovingModel(center,radius*0.8f);
    
    osg::Node* movingModel = ret_array[0];
	
	osg::Group* test_model = new osg::Group;

    const bool add_planes = true;
    if (add_planes)
    {
		//osg::Node* p_copy = dynamic_cast<osg::Node*>(ret_array[1]->clone(osg::CopyOp::DEEP_COPY_ALL)); 
        //findNodeVisitor findNodes("white_blink"); 
		//p_copy->accept(findNodes);

	    //findNodeVisitor::nodeListType& wln_list = findNodes.getNodeList();
		
		//for(auto it = wln_list.begin(); it != wln_list.end(); ++it )
		//{
		//	(*it)->setUpdateCallback(new effects::BlinkNode(white_color,black_color));
		//}
		nodes_array_t plane = loadAirplaneParts();
		auto p_copy = plane[1];

#if 1
        //effects::createShader(p_copy/*geom*/) ;
        effects::createProgram(p_copy,circles::vs,circles::fs) ;
#endif 

		const unsigned inst_num = 12;
        for (unsigned i = 0; i < inst_num; ++i)
        {
            float const angle = 2.0f * /*cg::pif*/osg::PI * i / inst_num, radius = 200.f;
            /*cg::point_3f*/ osg::Vec3 pos(radius * sin (angle), radius * cos(angle), 0.f);

            //cg::transform_4f rottrans = cg::transform_4f(
            //    as_translation(cg::point_3f(pos)),
            //    cg::cprf(180.f * (i & 1) + cg::rad2grad(angle), 0.f, 0.f),
            //    as_scale(cg::point_3f(1.f, 1.f, 1.f)));

            //transform_node_ptr cur_trans = m_pVictory->scenegraph()->create(node::NT_Transform)->as_transform();
            //cur_trans->set_transform(rottrans);

			const osg::Quat quat(osg::inDegrees(-90.f), osg::X_AXIS,                      
                                 osg::inDegrees(0.f) , osg::Y_AXIS,
                                 osg::inDegrees(180.f * (i & 1)) - angle  , osg::Z_AXIS ); 


			osg::MatrixTransform* positioned = new osg::MatrixTransform(osg::Matrix::translate(pos));
			positioned->setDataVariance(osg::Object::STATIC);
			
			osg::MatrixTransform* rotated = new osg::MatrixTransform(osg::Matrix::rotate(quat));
			rotated->setDataVariance(osg::Object::STATIC);
			
			positioned->addChild(rotated);
			rotated->addChild(p_copy/*ret_array[1]*/);


            //node_ptr ptrCessnaNode = m_pVictory->scenegraph()->load("learjet60/learjet60.scg");
            //cur_trans->add(ptrCessnaNode.get());



            // add it
            // m_pScene->get_objects()->add(cur_trans.get());
			//if(i%2==0) 
				root->addChild(positioned);
			//else
			//	movingModel->asGroup()->addChild(positioned);
        }
    }

    if (overlay)
    {
        osgSim::OverlayNode* overlayNode = new osgSim::OverlayNode(technique);
        overlayNode->setContinuousUpdate(true);
        overlayNode->setOverlaySubgraph(movingModel);
        overlayNode->setOverlayBaseHeight(baseHeight-0.01);
        overlayNode->addChild(baseModel);
        root->addChild(overlayNode);
    }
    else
    {

        root->addChild(baseModel);
    }
    
    movingModel->setName("moving_model");

#ifdef TEST_SHADOWS
	osg::ref_ptr<osgShadow::ShadowedScene> shadowScene
		= new osgShadow::ShadowedScene;
	osg::ref_ptr<osgShadow::ShadowMap> sm = new osgShadow::ShadowMap;
	shadowScene->setShadowTechnique(sm.get());
	//shadowScene->addChild(ret_array[1]);
	shadowScene->addChild(movingModel);
	root->addChild(shadowScene);
#else
    root->addChild(movingModel);
#endif



#ifdef SPOT_LIGHT
    //root->setStateSet(lights::createSpotLightDecoratorState(10,1));
#endif

    ret_array[0] = root;


    return ret_array;
}

} // ns creators