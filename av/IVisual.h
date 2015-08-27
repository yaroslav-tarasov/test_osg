#pragma once

struct IVisual 
{
    virtual void  Initialize() = 0;
    virtual void  Deinitialize() = 0;

    virtual void  Update() = 0;
    virtual void  Render() = 0;

    virtual double GetInternalTime() = 0;

    virtual void CreateScene() = 0; 
};

IVisual *  CreateVisual();