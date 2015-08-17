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

private:
    Visual();
    virtual  ~Visual();


private:
    bool            m_bInitialized;
    static Visual * m_pInstance;

};