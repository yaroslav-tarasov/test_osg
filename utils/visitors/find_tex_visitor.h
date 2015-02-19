#ifndef FIND_TEX_VISITOR_H
#define FIND_TEX_VISITOR_H


class ReplaceTextureVisitor : public osg::NodeVisitor
{
public:
    ReplaceTextureVisitor( osg::Texture* tex ) : _texture(tex)
    {
        setTraversalMode( osg::NodeVisitor::TRAVERSE_ALL_CHILDREN );
    }

    virtual void apply( osg::Node& node )
    {
        replaceTexture( node.getStateSet() );
        traverse( node );
    }

    virtual void apply( osg::Geode& geode )
    {
        replaceTexture( geode.getStateSet() );
        for ( unsigned int i=0; i<geode.getNumDrawables(); ++i )
        {
            replaceTexture( geode.getDrawable(i)->getStateSet() );
        }
        traverse( geode );
    }

    void replaceTexture( osg::StateSet* stateset )
    {
        if ( stateset )
        {

            size_t tl_s = stateset->getTextureAttributeList().size();

            for(unsigned i = 0;i< tl_s;i++)
            {
                if(_old_texture) break;

                osg::Texture* oldTexture = dynamic_cast<osg::Texture*>(
                    stateset->getTextureAttribute(i, osg::StateAttribute::TEXTURE) );

                if ( oldTexture )
                {  
                    std::string name_ = stateset->getName();
                    if(!_old_texture) _old_texture = new  osg::Texture2D(oldTexture->getImage(0));
                    stateset->setTextureAttribute( i, _texture.get() );

                }
            }
        }
    }

    inline osg::Texture* getOldTexture()
    {
        return  _old_texture.release();
    }

protected:
    osg::ref_ptr<osg::Texture> _texture;
    osg::ref_ptr<osg::Texture> _old_texture;
};


class FindTextureVisitor : public osg::NodeVisitor
{
public:
    FindTextureVisitor( std::string  texture_name ) : _texture_name(texture_name)
    {
        setTraversalMode( osg::NodeVisitor::TRAVERSE_ALL_CHILDREN );
    }

    virtual void apply( osg::Node& node )
    {
        findTexture( node.getStateSet() );
        traverse( node );
    }

    virtual void apply( osg::Geode& geode )
    {
        findTexture( geode.getStateSet() );
        for ( unsigned int i=0; i<geode.getNumDrawables(); ++i )
        {
            findTexture( geode.getDrawable(i)->getStateSet() );
        }
        traverse( geode );
    }

    void findTexture( osg::StateSet* stateset )
    {
        if ( stateset )
        {
            size_t tl_s = stateset->getTextureAttributeList().size();

            for(unsigned i = 0;i< tl_s;i++)
            {
                if(_found_texture) break;

                osg::Texture* foundTexture = dynamic_cast<osg::Texture*>(
                    stateset->getTextureAttribute(i, osg::StateAttribute::TEXTURE) );

                if ( foundTexture )
                {  
                    std::string name = stateset->getName();

                    bool matching = name.find(_texture_name) !=std::string::npos; 
                   
                    if(!_found_texture)
                        _found_texture = new  osg::Texture2D(foundTexture->getImage(0));
                }
            }
        }
    }

    inline osg::Texture* getTexture()
    {
        return  _found_texture.release();
    }

protected:
    std::string  _texture_name;
    osg::ref_ptr<osg::Texture> _found_texture;
};

#endif