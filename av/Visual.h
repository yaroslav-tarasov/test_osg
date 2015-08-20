#pragma once 
#include "IVisual.h"

class Visual : public IVisual
{
public:
    static Visual * CreateInstance();

    virtual void  Initialize(int argc, char** argv) override;
    virtual void  Deinitialize()                    override;

    virtual void  Update()                          override;
    virtual void  Render()                          override;

    virtual double GetInternalTime()                override;
    virtual void   CreateScene()                    override;

private:
    Visual();
    virtual  ~Visual();
    
    void InitializeViewer(const osg::ArgumentParser& cArgs, osg::ref_ptr<osg::GraphicsContext::Traits> cTraitsPtr);

private:
    bool            m_bInitialized;
    static Visual * m_pInstance;

private:
    osg::ref_ptr<osgViewer::Viewer>             _viewerPtr;

};