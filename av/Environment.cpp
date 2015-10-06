#include "stdafx.h"

#include "av/precompiled.h"

#include "av/Utils.h"
#include "av/Scene.h"


using namespace avCore;


Environment * Environment::m_pInstance = NULL;


//////////////////////////////////////////////////////////////////////////
Environment::Environment()
{
}

//////////////////////////////////////////////////////////////////////////
Environment::~Environment()
{
}

//////////////////////////////////////////////////////////////////////////
Environment * Environment::GetInstance()
{
    avAssert(m_pInstance);
    return m_pInstance;
}

//////////////////////////////////////////////////////////////////////////
void Environment::Create()
{
    avAssert(m_pInstance == NULL);
    m_pInstance = new Environment();
}

//////////////////////////////////////////////////////////////////////////
void Environment::Release()
{
    avAssert(m_pInstance != NULL);
    delete m_pInstance;
    m_pInstance = nullptr;
}


