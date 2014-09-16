class InfoVisitor : public osg::NodeVisitor
{
public:
    InfoVisitor()  
      :_level(0)
      , osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN)
    { }

    std::string spaces()
    { return std::string(_level*2, ' '); }

    void apply( osg::Node& node )
    {
        std::cout << spaces() << node.libraryName() << "::" << node.className() << " : " << node.getName() << std::endl;

        _level++;
        traverse( node );
        _level--;
    }

    void apply( osg::Geode& geode )
    {
        std::cout << spaces() << geode.libraryName() << "::" << geode.className() << " : " << geode.getName() << std::endl;

        _level++;
        for ( unsigned int i=0; i<geode.getNumDrawables(); ++i )
        {
            osg::Drawable* drawable = geode.getDrawable(i);
            std::cout << spaces() << drawable->libraryName() << "::" << drawable->className() << " : " << geode.getName() << std::endl;
        }

        traverse( geode );
        _level--;
    }

protected:
    unsigned int _level;
};