#pragma once 


namespace visual_objects
{
    struct morphs_support
    {
        void add(osg::Node * node)
        {
            morphs_.push_back(dynamic_cast<osgAnimation::MorphGeometry*>(node->asGeode()->getDrawable(0)->asGeometry()));
        }

        void set_weight(unsigned int index, float morphWeight)
        {
            for (auto it = morphs_.begin();it!=morphs_.end();++it)
            {
                (*it)->setWeight(index,morphWeight);
            }
        }

        void set_visibility(bool visible)
        {
            for (auto it = morphs_.begin();it!=morphs_.end();++it)
            {
                for(int i=0;i<(*(it))->getNumParents();++i)
                    (*(it))->getParent(i)->setNodeMask(visible?0x00010000:0); 
            }
        }

    private:

        std::vector<osgAnimation::MorphGeometry *>           morphs_;

    };

}