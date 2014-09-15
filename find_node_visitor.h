#ifndef FIND_NODE_VISITOR_H
#define FIND_NODE_VISITOR_H

#include <osg/NodeVisitor>
#include <osg/Node>

class findNodeVisitor : public osg::NodeVisitor 
{
public: 
    enum match_type_t {exact,not_exact};
    // Default constructor - initialize searchForName to "" and 
    // set the traversal mode to TRAVERSE_ALL_CHILDREN
    explicit findNodeVisitor(match_type_t m = exact); 

    // Constructor that accepts string argument
    // Initializes searchForName to user string
    // set the traversal mode to TRAVERSE_ALL_CHILDREN
    findNodeVisitor(const std::string &searchName, match_type_t m=exact);

    // The 'apply' method for 'node' type instances.
    // Compare the 'searchForName' data member against the node's name.
    // If the strings match, add this node to our list
    virtual void apply(osg::Node &searchNode);

    // Set the searchForName to user-defined string
    void setNameToFind(const std::string &searchName);

    // Return a pointer to the first node in the list
    // with a matching name
    osg::Node* getFirst();

    osg::Node* getLast();
    // typedef a vector of node pointers for convenience
    typedef std::vector<osg::Node*> nodeListType; 

    // return a reference to the list of nodes we found
    nodeListType& getNodeList() { return foundNodeList; }

private:

    // the name we are looking for
    std::string searchForName; 

    // List of nodes with names that match the searchForName string
    nodeListType foundNodeList;

    match_type_t m_;

};

#endif