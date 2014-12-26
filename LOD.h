#pragma once

namespace avLod {

typedef std::vector< osg::observer_ptr<osg::Node> > NodeList;

class /*OSG_EXPORT*/ LOD : public osg::Node
{
    public :

        LOD();

        /** Copy constructor using CopyOp to manage deep vs shallow copy.*/
        LOD(const LOD&,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY);

        META_Node(avLod, LOD);

        typedef osg::BoundingSphere::vec_type vec_type;
        typedef osg::BoundingSphere::value_type value_type;

        virtual void traverse(osg::NodeVisitor& nv);

        virtual bool addChild(osg::Node *child);

        virtual bool addChild(osg::Node *child, float min, float max);

        virtual bool removeChildren(unsigned int pos,unsigned int numChildrenToRemove=1);

        typedef std::pair<float,float>  MinMaxPair;
        typedef std::vector<MinMaxPair> RangeList;

        /** Modes which control how the center of object should be determined when computing which child is active.*/
        enum CenterMode
        {
            USE_BOUNDING_SPHERE_CENTER,
            USER_DEFINED_CENTER,
            UNION_OF_BOUNDING_SPHERE_AND_USER_DEFINED
        };

        /** Set how the center of object should be determined when computing which child is active.*/
        void setCenterMode(CenterMode mode) { _centerMode=mode; }

        /** Get how the center of object should be determined when computing which child is active.*/
        CenterMode getCenterMode() const { return _centerMode; }

        /** Sets the object-space point which defines the center of the osg::LOD.
            center is affected by any transforms in the hierarchy above the osg::LOD.*/
        inline void setCenter(const vec_type& center) { if (_centerMode!=UNION_OF_BOUNDING_SPHERE_AND_USER_DEFINED) { _centerMode=USER_DEFINED_CENTER; } _userDefinedCenter = center; }

        /** return the LOD center point. */
        inline const vec_type& getCenter() const { if ((_centerMode==USER_DEFINED_CENTER)||(_centerMode==UNION_OF_BOUNDING_SPHERE_AND_USER_DEFINED)) return _userDefinedCenter; else return getBound().center(); }


        /** Set the object-space reference radius of the volume enclosed by the LOD.
          * Used to determine the bounding sphere of the LOD in the absence of any children.*/
        inline void setRadius(value_type radius) { _radius = radius; }

        /** Get the object-space radius of the volume enclosed by the LOD.*/
        inline value_type getRadius() const { return _radius; }

        /** Modes that control how the range values should be interpreted when computing which child is active.*/
        enum RangeMode
        {
            DISTANCE_FROM_EYE_POINT,
            PIXEL_SIZE_ON_SCREEN
        };

        /** Set how the range values should be interpreted when computing which child is active.*/
        void setRangeMode(RangeMode mode) { _rangeMode = mode; }

        /** Get how the range values should be interpreted when computing which child is active.*/
        RangeMode getRangeMode() const { return _rangeMode; }


        /** Sets the min and max visible ranges of range of specific child.
            Values are floating point distance specified in local objects coordinates.*/
        void setRange(unsigned int childNo, float min,float max);

        /** returns the min visible range for specified child.*/
        inline float getMinRange(unsigned int childNo) const { return _rangeList[childNo].first; }

        /** returns the max visible range for specified child.*/
        inline float getMaxRange(unsigned int childNo) const { return _rangeList[childNo].second; }

        /** returns the number of ranges currently set.
          * An LOD which has been fully set up will have getNumChildren()==getNumRanges(). */
        inline unsigned int getNumRanges() const { return static_cast<unsigned int>(_rangeList.size()); }

        /** set the list of MinMax ranges for each child.*/
        inline void setRangeList(const RangeList& rangeList) { _rangeList=rangeList; }

        /** return the list of MinMax ranges for each child.*/
        inline const RangeList& getRangeList() const { return _rangeList; }

        virtual osg::BoundingSphere computeBound() const;

    protected :
        virtual ~LOD() {}

        CenterMode                      _centerMode;
        vec_type                        _userDefinedCenter;
        value_type                      _radius;

        RangeMode                       _rangeMode;
        RangeList                       _rangeList;
        NodeList                        _children;
        bool                            _dirty_copy;
};

}

