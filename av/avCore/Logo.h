#pragma once 

namespace avScene 
{
   namespace Logo
   {
       bool Create( osgViewer::Viewer* vw );
       osg::Node* Create(const std::string& filename, const std::string& label, const std::string& subscript);
   }
}
