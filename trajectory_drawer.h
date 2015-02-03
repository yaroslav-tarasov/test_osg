#pragma once 

class TrajectoryDrawer : public osg::Group
{
public:
    TrajectoryDrawer()
        : vert_(new osg::Vec3Array)
        , traj_lines_(new osg::DrawElementsUInt(osg::PrimitiveSet::POINTS, 0))
    {
        auto geode_ = new osg::Geode();
        addChild(geode_);

        geometry_ = new osg::Geometry();
        geode_->addDrawable(geometry_);
        geometry_->getOrCreateStateSet()->setAttribute( new osg::Point( 5.0f ),
            osg::StateAttribute::ON );
        geometry_->getOrCreateStateSet()->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
        
    }

    void set(fms::trajectory::keypoints_t& kp)
    {
       kp_ =  kp;
       vert_->clear();
       traj_lines_->clear();

       int i=0;
       BOOST_FOREACH(const auto &p, kp.points())
       {
           vert_->push_back( osg::Vec3( p.second.x, p.second.y, 0.8) );
           traj_lines_->push_back(i++);
       } 
       
       geometry_->setVertexArray( vert_ );
       geometry_->addPrimitiveSet(traj_lines_);

       osg::Vec4Array* colors = new osg::Vec4Array;
       colors->push_back(osg::Vec4(1.0f, 0.0f, 0.0f, 1.0f) ); 
       geometry_->setColorArray(colors);
       geometry_->setColorBinding(osg::Geometry::BIND_OVERALL);

    }
    //
    // OSG node interfaces
    //

    // cloning not assumed
    osg::Object * cloneType()                           const { return NULL; }
    osg::Object * clone    (const osg::CopyOp & copyop) const { return NULL; }

    // META_Node realization
    bool         isSameKindAs(const osg::Object * obj) const { return static_cast<const TrajectoryDrawer *>(obj) != NULL; }
    const char * className   ()                        const { return "TrajectoryDrawer"; }
    const char * libraryName ()                        const { return "avScene"; }

private:
    fms::trajectory::keypoints_t                         kp_;
    osg::ref_ptr<osg::Vec3Array>                       vert_; 
    osg::ref_ptr<osg::Geometry>                    geometry_;
    osg::ref_ptr<osg::DrawElementsUInt>          traj_lines_;
};
