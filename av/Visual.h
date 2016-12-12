#pragma once 
#include "av/IVisual.h"

namespace av
{



class  Visual : public IVisual
{
public:
    static Visual * CreateInstance();

    void   Initialize()                      override;
    void   Deinitialize()                    override;

    void   Update(double ref_time)           override;
    void   Render(double ref_time)           override;
    
	void   SetFreeViewControl(IFreeViewControl * fvc) override; 

    double GetInternalTime ()               override;
    void   CreateScene     ()               override;
    void   EndSceneCreation()               override;
                                        
    bool   Done   ()                        override;
    void   SetDone(bool )                   override;

    IScenePtr          GetScene () const    override;
    osgViewer::Viewer* GetViewer() const    override;

//  viewport	
	virtual void   SetPosition (const cg::point_3f&, const cg::quaternion&) override ;

private:
    Visual();
    virtual  ~Visual();
    
    void InitializeViewer(osg::ref_ptr<osg::GraphicsContext::Traits> cTraitsPtr);
    void PreUpdate       (double ref_time);
private:
    bool            m_bInitialized;
    static Visual * m_pInstance;

private:
    osg::ref_ptr<osgViewer::Viewer>             _viewerPtr;
};

}