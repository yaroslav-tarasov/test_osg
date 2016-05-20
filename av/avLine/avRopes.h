 #pragma once

#include "objects/arresting_gear.h"

//
//  RopesNode interface
//

// RopesNode
struct IRopesNode 
{
    virtual void  updateRopes( const arresting_gear::ropes_state_t& ) = 0;
};


typedef  IRopesNode*  RopesNodePtr;