#ifndef FIND_NODE_VISITOR_H
#define FIND_NODE_VISITOR_H

class findNodeVisitor : public osg::NodeVisitor 
{
public: 
    enum match_type_t {exact,not_exact};

    typedef std::list<std::string>  nodeNamesList; 
    // typedef a vector of node pointers for convenience
    typedef std::vector<osg::Node*> nodeListType; 

    // Default constructor - initialize searchForName to "" and 
    // set the traversal mode to TRAVERSE_ALL_CHILDREN
    explicit findNodeVisitor(match_type_t m = exact); 

    // Constructor that accepts string argument
    // Initializes searchForName to user string
    // set the traversal mode to TRAVERSE_ALL_CHILDREN
    findNodeVisitor(const std::string &searchName, match_type_t m=exact);

    findNodeVisitor(const nodeNamesList &searchNames, match_type_t m=exact);

    // The 'apply' method for 'node' type instances.
    // Compare the 'searchForName' data member against the node's name.
    // If the strings match, add this node to our list
    virtual void apply(osg::Node &searchNode);

    // Set the searchForName to user-defined string
    void setNameToFind(const std::string &searchName);

    // Set the searchForName to user-defined string
    void setNamesToFind(const nodeNamesList &searchNames );
    
    // Return a pointer to the first node in the list
    // with a matching name
    osg::Node* getFirst();

    osg::Node* getLast();


    // return a reference to the list of nodes we found
    nodeListType& getNodeList() { return foundNodeList; }

private:

    // the name we are looking for
    nodeNamesList searchForName; 

    // List of nodes with names that match the searchForName string
    nodeListType foundNodeList;

    match_type_t m_;

};

template<typename T>
class findNodeByType : public osg::NodeVisitor 
{
public: 
    // typedef a vector of node pointers for convenience
    typedef std::vector<osg::Node*> nodeListType; 

    typedef T nodeType; 

    findNodeByType()
        : osg::NodeVisitor(TRAVERSE_ALL_CHILDREN)
    {    
    }  

    virtual void apply(osg::Node &searchNode)
    {    
        if (dynamic_cast<T*>(&searchNode))   
        {   
            foundNodeList.push_back(&searchNode);   
        }   
        traverse(searchNode);    
    } 

    // Return a pointer to the first node in the list
    // with a matching name
    osg::Node* getFirst()   
    {   
        if(foundNodeList.size()>0) 
            return *(foundNodeList.begin());
        else
            return nullptr;
    }  

    osg::Node* getLast()   
    {   
        if(foundNodeList.size()>0) 
            return foundNodeList.back();
        else
            return nullptr;
    } 


    // return a reference to the list of nodes we found
    nodeListType& getNodeList() { return foundNodeList; }

private:

    // List of nodes with names that match the searchForName string
    nodeListType foundNodeList;

};

inline static osg::Node * findFirstNode(osg::Node* root,const std::string &searchName, findNodeVisitor::match_type_t m=findNodeVisitor::exact)
{
    findNodeVisitor findVis(searchName,m); 
    root->accept(findVis);
    return findVis.getFirst();
}

#endif