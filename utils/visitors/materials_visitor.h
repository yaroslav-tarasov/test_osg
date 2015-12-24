#pragma once

namespace mat
{
    struct texture_t
    {
        int         unit;
        std::string path;
        osg::Texture::WrapMode wrap_s;
        osg::Texture::WrapMode wrap_t;
    };

    typedef std::multimap<std::string,texture_t> materials_t;
}

class MaterialVisitor : public osg::NodeVisitor
{
public:         
    typedef std::function<void(osg::Node*, osg::StateSet* stateset,std::string,const mat::materials_t& m)>       creator_f;
    typedef std::function<void(osg::Node*,std::string)>                    computer_f;    
    typedef std::list<std::string>                                         namesList;
public:
    MaterialVisitor( const namesList &searchNames, creator_f cr , computer_f cm,const mat::materials_t& m) 
        : _cr(cr)
        , _cm(cm)
        , _mats(m)
        , _found_texture(false)
        , searchForName(searchNames)
    {
        setTraversalMode( osg::NodeVisitor::TRAVERSE_ALL_CHILDREN );
    }

    virtual void apply( osg::Node& node )
    {
        if(findTexture( &node, node.getStateSet() ))
        {   
            if(_cm) _cm(&node,_found_mat_name);
            if(_cr) _cr(&node, node.getStateSet(),_found_mat_name,_mats);
        }

        traverse( node );
    }

    virtual void apply( osg::Geode& geode )
    {
        bool ret = findTexture( &geode,geode.getStateSet() );
        if (ret)
            _cr(&geode,geode.getStateSet(),_found_mat_name,_mats);
        for ( unsigned int i=0; i<geode.getNumDrawables(); ++i )
        {
            ret |= findTexture( &geode, geode.getDrawable(i)->getStateSet() );
            if (ret)
                _cr(&geode, geode.getDrawable(i)->getStateSet(),_found_mat_name,_mats);
        }

        if(ret && _cm)
            _cm(&geode,_found_mat_name);


        traverse( geode );
    }

    bool findTexture( osg::Node * node, osg::StateSet* stateset )
    {
        bool ret = false;
        if ( stateset )
        {
            size_t tl_s = stateset->getTextureAttributeList().size();

            for(unsigned i = 0;i< tl_s;i++)
            {
                osg::Texture* foundTexture = dynamic_cast<osg::Texture*>(
                    stateset->getTextureAttribute(i, osg::StateAttribute::TEXTURE) );

                if ( foundTexture )
                {  
                    std::string name = stateset->getName();

                    bool matching = searchForName.end() != std::find_if(searchForName.begin(),searchForName.end(),[&](const std::string& name_)->bool {
                        return name.find(name_) !=std::string::npos; }
                    );

                    if(matching)
                    {  
                        _found_mat_names.insert(name);
                        _found_mat_name = name;
                    }

                    ret =true;
                }
            }
        }

        return ret;
    }

    inline std::set<std::string> getFoundTextureName()
    {
        return   _found_mat_names;
    }

protected:
    std::set<std::string>  _found_mat_names;
    std::string            _found_mat_name;
    creator_f              _cr;
    computer_f             _cm; 
    mat::materials_t       _mats;
    bool                   _found_texture;

    // the name we are looking for
    namesList searchForName; 
};

