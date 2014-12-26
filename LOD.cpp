#include "stdafx.h"
#include "LOD.h"
#include <osg/CullStack>

#include <algorithm>

using namespace avLod;

LOD::LOD()
    :_centerMode(USE_BOUNDING_SPHERE_CENTER)
    ,_radius(-1.0f)
    ,_rangeMode(DISTANCE_FROM_EYE_POINT)
    ,_dirty_copy(false)
{

}

LOD::LOD(const LOD& lod,const osg::CopyOp& copyop)
        :osg::Node(lod,copyop)
        ,_centerMode(lod._centerMode)
        ,_userDefinedCenter(lod._userDefinedCenter)
        ,_radius(lod._radius)
        ,_rangeMode(lod._rangeMode)
        ,_rangeList(lod._rangeList)
        ,_dirty_copy(true)
{

}


void LOD::traverse(osg::NodeVisitor& nv)
{
    if(_dirty_copy)
    {
        osg::Group* prnt =  getParent(0);
        for(int i =0;i<prnt->getNumChildren();++i)
        {
            auto chld = prnt->getChild(i);
            if (!dynamic_cast<avLod::LOD*>(chld))
                addChild(chld);
        }
        _dirty_copy =false;
    }
    switch(nv.getTraversalMode())
    {
        //case(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN):
        //    std::for_each(_children.begin(),_children.end(),NodeAcceptOp(nv));
        //    break;
        case(osg::NodeVisitor::TRAVERSE_ACTIVE_CHILDREN):
        {
            float required_range = 0;
            if (_rangeMode==DISTANCE_FROM_EYE_POINT)
            {
                required_range = nv.getDistanceToViewPoint(getCenter(),true);
            }
            else
            {
                osg::CullStack* cullStack = dynamic_cast<osg::CullStack*>(&nv);
                if (cullStack && cullStack->getLODScale())
                {
                    required_range = cullStack->clampedPixelSize(getBound()) / cullStack->getLODScale();
                }
                else
                {
                    // fallback to selecting the highest res tile by
                    // finding out the max range
                    for(unsigned int i=0;i<_rangeList.size();++i)
                    {
                        required_range = osg::maximum(required_range,_rangeList[i].first);
                    }
                }
            }

            unsigned int numChildren = _children.size();
            if (_rangeList.size()<numChildren) numChildren=_rangeList.size();

            for(unsigned int i=0;i<numChildren;++i)
            {
                if (_rangeList[i].first<=required_range && required_range<_rangeList[i].second)
                {
                    _children[i]->setNodeMask(0xffffffff);// accept(nv);
                }
                else
                {
                    _children[i]->setNodeMask(0);
                }
            }
           break;
        }
        default:
            break;
    }
}

osg::BoundingSphere LOD::computeBound() const
{
    if (_centerMode==USER_DEFINED_CENTER && _radius>=0.0f)
    {
        return osg::BoundingSphere(_userDefinedCenter,_radius);
    }
    else if (_centerMode==UNION_OF_BOUNDING_SPHERE_AND_USER_DEFINED && _radius>=0.0f)
    {
        osg::BoundingSphere bs = osg::BoundingSphere(_userDefinedCenter,_radius);
        bs.expandBy(/*osg::Group*/osg::Node::computeBound());
        return bs;
    }
    else
    {
        return /*Group::*/osg::Node::computeBound();
    }
}

bool LOD::addChild( Node *child )
{
    _children.push_back(child);
    if (/*Group::addChild(child)*/true)
    {

        if (_children.size()>_rangeList.size())
        {
            float maxRange = !_rangeList.empty() ? _rangeList.back().second : 0.0f;

            _rangeList.resize(_children.size(),MinMaxPair(maxRange,maxRange));
        }

        return true;
    }
    return false;
}


bool LOD::addChild(Node *child, float min, float max)
{
    _children.push_back(child);
    if (/*Group::addChild(child)*/true)
    {
        if (_children.size()>_rangeList.size()) _rangeList.resize(_children.size(),MinMaxPair(min,min));
        _rangeList[_children.size()-1].first = min;
        _rangeList[_children.size()-1].second = max;
        return true;
    }
    return false;
}

bool LOD::removeChildren( unsigned int pos,unsigned int numChildrenToRemove)
{
    //if (pos<_rangeList.size()) _rangeList.erase(_rangeList.begin()+pos, osg::minimum(_rangeList.begin()+(pos+numChildrenToRemove), _rangeList.end()) );

    //return Group::removeChildren(pos,numChildrenToRemove);
    return false;
}

void LOD::setRange(unsigned int childNo, float min,float max)
{
    if (childNo>=_rangeList.size()) _rangeList.resize(childNo+1,MinMaxPair(min,min));
    _rangeList[childNo].first=min;
    _rangeList[childNo].second=max;
}
