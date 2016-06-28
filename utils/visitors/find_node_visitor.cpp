#include "stdafx.h"

#include "find_node_visitor.h"   
#include "cpp_utils/str.h"

using namespace std::placeholders;

namespace 
{
    std::list<std::string>  to_lower (std::list<std::string> &sn)
    {
        std::list<std::string> out;
        for (auto it = sn.begin();it!=sn.end();++it )
        {
            out.push_back(boost::to_lower_copy(*it));
        }

        // Yoy yoy
        //1>c:\work\prj\test_osg\find_node_visitor.cpp(46): fatal error C1001: An internal error has occurred in the compiler.
        //1>  (compiler file 'f:\dd\vctools\compiler\utc\src\p2\ehexcept.c', line 971)
        //1>   To work around this problem, try simplifying or changing the program near the locations listed above.
        //1>  Please choose the Technical Support command on the Visual C++ 
        //1>   Help menu, or open the Technical Support help file for more information

        //std::for_each(sn.begin(),sn.end(), [&](const std::string & name){
        //    std::string o = boost::to_lower_copy(name);
        //    out.push_back(o);
        //});

        return out;
    }
}

// Default constructor - initialize searchForName to "" and    
// set the traversal mode to TRAVERSE_ALL_CHILDREN   
FindNodeVisitor::FindNodeVisitor(match_type_t m, osg::NodeVisitor::TraversalMode tm, search_type_t st) 
    : osg::NodeVisitor(tm)
    , searchForName()
    , m_(m)
    , st_(st)
{    
}    

// Constructor that accepts string argument   
// Initializes searchForName to user string   
// set the traversal mode to TRAVERSE_ALL_CHILDREN   
FindNodeVisitor::FindNodeVisitor(const std::string &searchName, match_type_t m,osg::NodeVisitor::TraversalMode tm, search_type_t st ) 
    : osg::NodeVisitor(tm)    
    , m_(m)
    , st_(st)
{    
    searchForName.push_back(boost::to_lower_copy(searchName));
}    

FindNodeVisitor::FindNodeVisitor(const std::list<std::string> &searchNames, match_type_t m, osg::NodeVisitor::TraversalMode tm, search_type_t st) 
    : osg::NodeVisitor(tm)    
    , searchForName(searchNames)
    , m_(m)
    , st_(st)
{    
}    

//The 'apply' method for 'node' type instances.   
//Compare the 'searchForName' data member against the node's name.   
//If the strings match, add this node to our list   
void FindNodeVisitor::apply(osg::Node &searchNode)    
{    
    bool matching = false;
    const std::string value = st_ == user_id?([&searchNode]()->std::string{std::string name; searchNode.getUserValue("dae_node_id", name); return name;}()):searchNode.getName(); 
    
    if(m_==exact)
    {
         matching = searchForName.end() != std::find(searchForName.begin(),searchForName.end(),boost::to_lower_copy(value));

		 //matching = searchForName.end() != std::find_if(
			// searchForName.begin(),
			// searchForName.end(),
			// std::bind(&utils::ci_find_substr_bool<std::string>,_1,searchNode.getName()));
	}
    else
    {
         matching = searchForName.end() != std::find_if(searchForName.begin(),searchForName.end(),
             [&](const std::string& nodeName)->bool {
             std::string ls = boost::to_lower_copy(value);
             return ls.find(nodeName) !=std::string::npos;
         }
         );
    }
    
    if (matching)   
    {   
        foundNodeList.push_back(&searchNode);   
    }   
    traverse(searchNode);    
}    

// Set the searchForName to user-defined string   
void FindNodeVisitor::setNameToFind(const std::string &searchName)    
{    
    searchForName.clear();
    searchForName.push_back(boost::to_lower_copy(searchName));    
    foundNodeList.clear();    
}    

// Set the searchForName to user-defined string
void FindNodeVisitor::setNamesToFind(const std::list<std::string> &searchNames )
{
    searchForName = searchNames;
    std::for_each(searchForName.begin(),searchForName.end(),[=](std::string& str){str = boost::to_lower_copy(str);});
    foundNodeList.clear();
}

osg::Node* FindNodeVisitor::getFirst()   
{   
    if(foundNodeList.size()>0) 
        return *(foundNodeList.begin());
    else
        return nullptr;
}  

osg::Node* FindNodeVisitor::getLast()   
{   
    if(foundNodeList.size()>0) 
        return foundNodeList.back();
    else
        return nullptr;
} 
