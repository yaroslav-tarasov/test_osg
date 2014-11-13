#pragma once

class ComputeTangentVisitor : public osg::NodeVisitor
{
public:
    void apply( osg::Node& node ) { traverse(node); }

    void apply( osg::Geode& node )
    {
        for ( unsigned int i=0; i<node.getNumDrawables(); ++i )
        {
            osg::Geometry* geom = dynamic_cast<osg::Geometry*>( node.getDrawable(i) );
            if ( geom ) generateTangentArray( geom );
        }
        traverse( node );
    }

    void generateTangentArray( osg::Geometry* geom )
    {
        osg::ref_ptr<osgUtil::TangentSpaceGenerator> tsg = new osgUtil::TangentSpaceGenerator;

        unsigned int tca = geom->getNumTexCoordArrays();
        for(unsigned i=0;i<tca;i++)
        {
            if(geom->getTexCoordArray(i))
            {
                tsg->generate( geom, /*1*/i );
                geom->setVertexAttribArray( 6, static_cast<osg::Array*>(tsg->getTangentArray()) );
                geom->setVertexAttribBinding( 6, osg::Geometry::BIND_PER_VERTEX );
                geom->setVertexAttribArray( 7, static_cast<osg::Array*>(tsg->getBinormalArray()) );
                geom->setVertexAttribBinding( 7, osg::Geometry::BIND_PER_VERTEX );
            }

        }
    }
};