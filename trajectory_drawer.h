#pragma once 

class TrajectoryDrawer : public osg::Group
{
public:
    enum drawing_mode
    {
        POINTS,
        LINES
    };
public:
    TrajectoryDrawer(osg::Group* parent = nullptr,drawing_mode mode = POINTS )
        : vert_(new osg::Vec3Array)
    {
        auto geode_ = new osg::Geode();
        addChild(geode_);

        geometry_ = new osg::Geometry();
        geode_->addDrawable(geometry_);

        if(mode==POINTS)
            geometry_->getOrCreateStateSet()->setAttribute( new osg::Point( 5.0f ),
                osg::StateAttribute::ON );

        geometry_->getOrCreateStateSet()->setMode( GL_LIGHTING, osg::StateAttribute::OFF );

        if (parent)
            parent->addChild(this);        
    }

    void set(const fms::trajectory_ptr traj)
    {

       vert_->clear();
       if(traj_lines_.get())
           traj_lines_->clear();

       if(int s = geometry_->getNumPrimitiveSets()>0)
               geometry_->removePrimitiveSet(0,s);

       
       traj_lines_ = new osg::DrawElementsUInt(osg::PrimitiveSet::POINTS, 0);

       int i=0;
       BOOST_FOREACH(const auto &p, traj->extract_values())
       {
           vert_->push_back( osg::Vec3( p.x, p.y, 0.8) );
           traj_lines_->push_back(i++);
       } 
       
       geometry_->setVertexArray( vert_ );
       geometry_->addPrimitiveSet(traj_lines_);

       osg::Vec4Array* colors = new osg::Vec4Array;
       colors->push_back(osg::Vec4(1.0f, 0.0f, 0.0f, 1.0f) ); 
       geometry_->setColorArray(colors);
       geometry_->setColorBinding(osg::Geometry::BIND_OVERALL);

    }

    void set(const std::vector<cg::point_3>& kp)
    {
        vert_->clear();
        if(traj_lines_.get())
            traj_lines_->clear();

        if(int s = geometry_->getNumPrimitiveSets()>0)
            geometry_->removePrimitiveSet(0,s);

        auto traj_lines_ = new osg::DrawElementsUInt(osg::PrimitiveSet::LINES, 0);

        int i=0;
        BOOST_FOREACH(const auto &p, kp)
        {   
            if(i!=0) traj_lines_->push_back(i);
            vert_->push_back( osg::Vec3( p.x, p.y, p.z + 0.1) );
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
    osg::ref_ptr<osg::Vec3Array>                       vert_; 
    osg::ref_ptr<osg::Geometry>                    geometry_;
    osg::ref_ptr<osg::DrawElementsUInt>          traj_lines_;
};
