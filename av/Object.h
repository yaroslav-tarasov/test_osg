#pragma once

namespace creators 
{
      osg::Node* createObject(std::string name, bool fclone=true);
      void       releaseObjectCache();
}