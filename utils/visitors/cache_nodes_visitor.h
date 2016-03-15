#pragma once

class CacheNodesVisitor : public osg::NodeVisitor 
{
    public:
        typedef std::unordered_map<std::string,osg::ref_ptr<osg::Node>> CacheMapType;

    public:
        explicit CacheNodesVisitor(CacheMapType& m, osg::NodeVisitor::TraversalMode tm = osg::NodeVisitor::TRAVERSE_ALL_CHILDREN)
            : osg::NodeVisitor(tm)
            , cache_ (m){}

        virtual void apply(osg::Node &searchNode)  
        {    
            std::string value;
            bool ret = searchNode.getUserValue("dae_node_id", value);
            
            if ( ret && value != searchNode.getName())
                cache_.insert(std::make_pair(value,&searchNode));
            else
               cache_.insert(std::make_pair(searchNode.getName(),&searchNode));

            traverse(searchNode);    
        } 
    private:

        CacheMapType&                             cache_;
};
