#include "stdafx.h"

namespace effects 
{ 
    
    osg::Vec3 wind(10.0f,0.0f,0.0f); 

    void insertParticle(osg::Group* root,osg::Group* rootModel, const osg::Vec3& center, float radius)
    {
        bool handleMovingModels = false;

        osg::Vec3 position = center + 
            osg::Vec3( radius * (((float)rand() / (float)RAND_MAX)-0.5)*2.0,
            radius * (((float)rand() / (float)RAND_MAX)-0.5)*2.0,
            0.0f);

        float scale = 10.0f * ((float)rand() / (float)RAND_MAX);
        float intensity = 1.0f;

        osgParticle::ExplosionEffect* explosion = new osgParticle::ExplosionEffect(position, scale, intensity);
        osgParticle::ExplosionDebrisEffect* explosionDebri = new osgParticle::ExplosionDebrisEffect(position, scale, intensity);
        osgParticle::FireEffect* fire = new osgParticle::FireEffect(position, scale, intensity);
        osgParticle::ParticleEffect* smoke = 0;
        //if (handleMovingModels)
            smoke =  new osgParticle::SmokeTrailEffect(position, scale, intensity);
        //else
        //    smoke =  new osgParticle::SmokeEffect(position, scale, intensity);

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

        // finally insert the particle systems into a Geode and attach to the root of the scene graph so the particle system
        // can be rendered.
        osg::Geode* geode = new osg::Geode;
        geode->addDrawable(explosion->getParticleSystem());
        geode->addDrawable(explosionDebri->getParticleSystem());
        geode->addDrawable(smoke->getParticleSystem());
        geode->addDrawable(fire->getParticleSystem());
        
        root->addChild(geode);

        rootModel->addChild(effectsGroup);
    }


}



namespace creators 
{

osg::AnimationPath* createAnimationPath(const osg::Vec3& center,float radius,double looptime)
{
    // set up the animation path
    osg::AnimationPath* animationPath = new osg::AnimationPath;
    animationPath->setLoopMode(osg::AnimationPath::LOOP);

    int numSamples = 40;
    float yaw = 0.0f;
    float yaw_delta = 2.0f*osg::PI/((float)numSamples-1.0f);
    float roll = osg::inDegrees(30.0f);

    double time=0.0f;
    double time_delta = looptime/(double)numSamples;
    for(int i=0;i<numSamples;++i)
    {
        osg::Vec3 position(center+osg::Vec3(sinf(yaw)*radius,cosf(yaw)*radius,0.0f));
        osg::Quat rotation(osg::Quat(roll,osg::Vec3(0.0,1.0,0.0))*osg::Quat(-(yaw+osg::inDegrees(90.0f)),osg::Vec3(0.0,0.0,1.0)));

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

    return geode;
}

osg::Node* createMovingModel(const osg::Vec3& center, float radius)
{
    float animationLength = 10.0f;

    osg::AnimationPath* animationPath = createAnimationPath(center,radius,animationLength);

    osg::Group* model = new osg::Group;

    osg::Node* glider = osgDB::readNodeFile("glider.osgt");
    if (glider)
    {
        const osg::BoundingSphere& bs = glider->getBound();

        float size = radius/bs.radius()*0.3f;
        osg::MatrixTransform* positioned = new osg::MatrixTransform;
        positioned->setDataVariance(osg::Object::STATIC);
        positioned->setMatrix(osg::Matrix::translate(-bs.center())*
            osg::Matrix::scale(size,size,size)*
            osg::Matrix::rotate(osg::inDegrees(-90.0f),0.0f,0.0f,1.0f));

        positioned->addChild(glider);

        osg::PositionAttitudeTransform* xform = new osg::PositionAttitudeTransform;
        xform->setUpdateCallback(new osg::AnimationPathCallback(animationPath,0.0,1.0));
        xform->addChild(positioned);

        model->addChild(xform);
    }

    osg::Node* cessna = osgDB::readNodeFile("an_124.dae");
    
    if (cessna)
    {
        cessna->setName("airplane");

        const osg::BoundingSphere& bs = cessna->getBound();

        const osg::Quat quat0(osg::inDegrees(-90.0f), osg::X_AXIS,                      
                              osg::inDegrees(0.0f)  , osg::Y_AXIS,
                              osg::inDegrees(0.0f)  , osg::Z_AXIS ); 

        float size = radius/bs.radius()*0.7f;
        osg::MatrixTransform* positioned = new osg::MatrixTransform;
        positioned->setDataVariance(osg::Object::DYNAMIC);
        positioned->setMatrix(osg::Matrix::translate(-bs.center())*
            osg::Matrix::scale(size,size,size)*
            osg::Matrix::rotate(quat0));

        positioned->addChild(cessna);
        
        osg::MatrixTransform* xform = new osg::MatrixTransform;
        xform->setUpdateCallback(new osg::AnimationPathCallback(animationPath,0.0f,2.0));
        xform->addChild(positioned);

        model->addChild(xform);



    }



    return model;
}

osg::Node* createModel(bool overlay, osgSim::OverlayNode::OverlayTechnique technique)
{
    osg::Vec3 center(0.0f,0.0f,0.0f);
    float radius = 100.0f;

    osg::Group* root = new osg::Group;

    float baseHeight = center.z()-radius*0.5;
    osg::Node* baseModel = createBase(osg::Vec3(center.x(), center.y(), baseHeight),radius);
    osg::Node* movingModel = createMovingModel(center,radius*0.8f);

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

    root->addChild(movingModel);

    return root;
}

} // ns creators