#pragma once 


class SmokeNode : public osg::Geode
{
public:
    SmokeNode();
    SmokeNode( const SmokeNode& copy,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY );
    META_Object( osg, SmokeNode )
    
	void setGravity (const osg::Vec3f& g);
    
protected:
	void    traverse(osg::NodeVisitor& nv) override;
    virtual ~SmokeNode();

private:
    boost::optional<osg::Vec3f> 		 gravity_;
	struct SafeEnvironment;
	boost::scoped_ptr<SafeEnvironment>   env_;
};


