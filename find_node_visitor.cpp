#include "stdafx.h"

#include "find_node_visitor.h"   

// Default constructor - initialize searchForName to "" and    
// set the traversal mode to TRAVERSE_ALL_CHILDREN   
findNodeVisitor::findNodeVisitor(match_type_t m) 
    : osg::NodeVisitor(TRAVERSE_ALL_CHILDREN)
    , searchForName()
    , m_(m) 
{    
}    

// Constructor that accepts string argument   
// Initializes searchForName to user string   
// set the traversal mode to TRAVERSE_ALL_CHILDREN   
findNodeVisitor::findNodeVisitor(const std::string &searchName, match_type_t m) 
    : osg::NodeVisitor(TRAVERSE_ALL_CHILDREN)    
    , m_(m)
{    
    searchForName.push_back(searchName);
}    

findNodeVisitor::findNodeVisitor(const std::list<std::string> &searchNames, match_type_t m) 
    : osg::NodeVisitor(TRAVERSE_ALL_CHILDREN)    
    , searchForName(searchNames)
    , m_(m)
{    
}    

//The 'apply' method for 'node' type instances.   
//Compare the 'searchForName' data member against the node's name.   
//If the strings match, add this node to our list   
void findNodeVisitor::apply(osg::Node &searchNode)    
{    
    bool matching = false;
    if(m_==exact)
    {
         matching = /*searchNode.getName()*/searchForName.end() != std::find(searchForName.begin(),searchForName.end(),searchNode.getName());
    }
    else
    {
         matching = searchForName.end() != std::find_if(searchForName.begin(),searchForName.end(),[&](const std::string& nodeName)->bool {
             return searchNode.getName().find(nodeName) !=std::string::npos; }
         );
         // matching = searchNode.getName().find(searchForName) !=std::string::npos ;
    }
    
    if (matching)   
    {   
        foundNodeList.push_back(&searchNode);   
    }   
    traverse(searchNode);    
}    

// Set the searchForName to user-defined string   
void findNodeVisitor::setNameToFind(const std::string &searchName)    
{    
    searchForName.clear();
    searchForName.push_back(searchName);    
    foundNodeList.clear();    
}    

// Set the searchForName to user-defined string
void findNodeVisitor::setNamesToFind(const std::list<std::string> &searchNames )
{
    searchForName = searchNames;
    foundNodeList.clear();
}

osg::Node* findNodeVisitor::getFirst()   
{   
    if(foundNodeList.size()>0) 
        return *(foundNodeList.begin());
    else
        return nullptr;
}  

osg::Node* findNodeVisitor::getLast()   
{   
    if(foundNodeList.size()>0) 
        return foundNodeList.back();
    else
        return nullptr;
} 