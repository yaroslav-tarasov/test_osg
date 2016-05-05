#include "stdafx.h"
#include "av/precompiled.h"

#include "av/avScene/Scene.h"
#include "Lights.h"

#include "LightManager.h"

#include "utils/callbacks.h"



namespace avScene
{


osg::ref_ptr<LightManager> LightManager::g_Instance = nullptr;


enum LightManagerMessage
{
    CreateDynamicLight,
    DeleteDynamicLight,
    UpdateDynamicLight,
};

typedef std::map<std::string, LightManagerMessage> LightManagerMessageMap;
typedef LightManagerMessageMap::value_type LightManagerMessageMapValue;

static const LightManagerMessageMapValue g_MessageTable[] =
{
    LightManagerMessageMapValue("CreateDynamicLight", CreateDynamicLight),
    LightManagerMessageMapValue("DeleteDynamicLight", DeleteDynamicLight),
    LightManagerMessageMapValue("UpdateDynamicLight", UpdateDynamicLight),
};
static const unsigned g_nMessageTableSize = sizeof(g_MessageTable) / sizeof(g_MessageTable[0]);
static const LightManagerMessageMap g_MessageMap(&g_MessageTable[0], &g_MessageTable[0] + g_nMessageTableSize);




LightManager::LightManager()
{
    setDataVariance(DYNAMIC);
    setUpdateCallback(Utils::makeNodeCallback(this, &LightManager::update));
}

LightManager::~LightManager()
{
}

void LightManager::Create()
{
    avAssert(!g_Instance.valid());
    g_Instance = new LightManager();
}

void LightManager::Release()
{
    avAssert(g_Instance.valid());
    g_Instance = NULL;
}

LightManager * LightManager::GetInstance()
{
    return g_Instance;
}


uint32_t  LightManager::genUID()
{
      uint32_t uid = 0;
      
      while (uid==0 || _LightsMap.find(uid)!=_LightsMap.end())
      {
          uid = cg::rand(cg::range_2(1, std::numeric_limits<uint32_t>::max()));
      };

      return uid;
}

uint32_t LightManager::addLight(const Light& data)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_lightManagerMapMutex);

	const unsigned lightID = genUID();

	Light & light = _LightsMap[lightID];

    light = data;
    
	light.active = true;

	return lightID;
}

void LightManager::update( osg::NodeVisitor * nv )
{
    avScene::Scene * scene = GetScene();

    if (scene == NULL)
        return;

    avScene::Lights * lights = scene->getLights();

    if (lights == NULL)
        return;
    
	OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_lightManagerMapMutex);

    for (LightsMap::const_iterator it = _LightsMap.cbegin(); it != _LightsMap.cend(); ++it)
    {
        const Light & light = it->second;

        if (!light.active)
            continue;
      
#if 0
        osg::Matrix matrix = light.transform->getMatrix() * utils::GetCoordinateSystem()->GetLCS2LTPMatrix();
        matrix4d transform = matrix4d(matrix.ptr(), matrix::unscaled).transpose();
#else
        
        osg::Matrix matrix; 
        
        if(light.transform)
        if(light.transform->asMatrixTransform())
        {
           osg::Node* parent = light.transform->getNumParents()>0?light.transform->getParent(0):nullptr;
           if(parent && parent->asTransform())
                matrix =  light.transform->asMatrixTransform()->getMatrix() * parent->asTransform()->asMatrixTransform()->getMatrix();
           else
                matrix =  light.transform->asMatrixTransform()->getMatrix() ;
        }

        cg::matrix_4 trm = from_osg_matrix(matrix);
        cg::transform_4 transform = trm.transpose();
#endif
        const cg::point_3f position = transform.treat_point(light.position );
        const cg::vector_3 direction = transform.treat_vector(light.direction);
        
        FIXME( "Ambient, Spec")
        lights->AddLight(avScene::/*GlobalInfluence*/LocalInfluence, avScene::ConicalLight, light.high_priority,
            position, direction, light.distanceFalloff, light.spotFalloff, 
            light.color, 0.60f, 0.35f);
    }


}


} // namespace avScene
