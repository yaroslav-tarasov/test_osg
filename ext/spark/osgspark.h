#pragma once

namespace spark
{   
    typedef std::pair<osg::Node*, osgGA::GUIEventHandler*> spark_pair_t;  
    enum spark_t {EXPLOSION,FIRE,RAIN,SMOKE,TEST};
    void init();
    spark_pair_t create(spark_t effectType,osg::Transform* model=nullptr);
}