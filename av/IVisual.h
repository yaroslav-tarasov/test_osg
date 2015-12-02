#pragma once

#include "visual/visual_api.h"

struct VISUAL_API IVisual 
{
    virtual void  Initialize()   = 0;
    virtual void  Deinitialize() = 0;

    virtual void  Update(double ref_time = -1.0)       = 0;
    virtual void  Render(double ref_time = -1.0)       = 0;
    
    virtual osgViewer::Viewer* GetViewer() const    =0;

    virtual double GetInternalTime() = 0;

    virtual void  CreateScene()      = 0;
    virtual void  EndSceneCreation() = 0;

    virtual bool   Done()            = 0;
    virtual void   SetDone(bool )    = 0;
};

VISUAL_API IVisual *  CreateVisual();