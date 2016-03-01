#pragma once 
#include "IVisual.h"

namespace av
{



class  Visual : public IVisual
{
public:
    static Visual * CreateInstance();

    virtual void   Initialize()                      override;
    virtual void   Deinitialize()                    override;

    virtual void   Update(double ref_time)           override;
    virtual void   Render(double ref_time)           override;

    virtual double GetInternalTime()                override;
    virtual void   CreateScene()                    override;
    virtual void   EndSceneCreation()               override;
                                        
    virtual bool   Done()                           override;
    virtual void   SetDone(bool )                   override;

    virtual IScenePtr          GetScene() const     override;
    virtual osgViewer::Viewer* GetViewer() const    override;

//  viewport	
	virtual void   SetPosition (const cg::point_3f&, const cg::quaternion&) override ;

private:
    Visual();
    virtual  ~Visual();
    
    void InitializeViewer(osg::ref_ptr<osg::GraphicsContext::Traits> cTraitsPtr);

private:
    bool            m_bInitialized;
    static Visual * m_pInstance;

private:
    osg::ref_ptr<osgViewer::Viewer>             _viewerPtr;

};

}