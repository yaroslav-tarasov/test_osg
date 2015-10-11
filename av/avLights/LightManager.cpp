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
    setUpdateCallback(utils::makeNodeCallback(this, &LightManager::update));
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
      
      while (uid==0 || m_LightsMap.find(uid)!=m_LightsMap.end())
      {
          uid = cg::rand(cg::range_2(1, std::numeric_limits<uint32_t>::max()));
      };

      return uid;
}


#if 0
uint32_t  LightManager::addLight(osg::MatrixTransform* mt )
{
    const unsigned lightID = genUID();
    
    Light & light = m_LightsMap[lightID];

    light.transform = mt;

    light.spotFalloff = cg::range_2f(cg::grad2rad(15.f), cg::grad2rad(45.f));

    const float distanceFalloff0 = 80.f;
    const float distanceFalloff1 = 220.f;
    light.distanceFalloff = cg::range_2f(distanceFalloff0, distanceFalloff1);

    light.color.r = 0.99;
    light.color.g = 0.99;
    light.color.b = 0.99;
    
    light.position = cg::point_3f(0,0,0);

    const float heading = osg::DegreesToRadians(0.f);
    const float pitch = osg::DegreesToRadians(/*-90.f*/0.);
    light.direction = as_vector(cg::point_3f(cos(pitch) * sin(heading), cos(pitch) * cos(heading), sin(pitch) ));

    light.active = true;

	return lightID;
}
#endif 


uint32_t LightManager::addLight(const Light& data)
{
    const unsigned lightID = genUID();

    Light & light = m_LightsMap[lightID];

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
     
    for (LightsMap::const_iterator it = m_LightsMap.cbegin(); it != m_LightsMap.cend(); ++it)
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
           osg::Node* parent = light.transform->getParent(0);
           if(parent->asTransform())
                matrix =  light.transform->asMatrixTransform()->getMatrix() * parent->asTransform()->asMatrixTransform()->getMatrix();
           else
                matrix =  light.transform->asMatrixTransform()->getMatrix() ;
        }

        cg::matrix_4 trm = from_osg_matrix(matrix);
        cg::transform_4 transform = trm.transpose();
#endif
        const cg::point_3f position = transform.treat_point(light.position );
        const cg::vector_3 direction = transform.treat_vector(light.direction);
        
        cg::range_2f spotFalloff = light.spotFalloff;

        FIXME(radians degrees)
#if 0
        //const cg::point_3f world_light_dir = mv_.treat_vector(spot.view_dir, false);
        //// angle corrected
        //auto corrected_spot = spot;
        //if (!corrected_spot.angle_falloff.empty())
        //    corrected_spot.angle_falloff |= cg::lerp01(spot.angle_falloff.hi(), 65.f, cg::bound(-world_light_dir.z, 0.f, 1.f));
        
        const cg::point_3f world_light_dir = transform.treat_vector(light.direction, false);
        // angle corrected

        if (!spotFalloff.empty())
            spotFalloff |= cg::lerp01(spotFalloff.hi(), 65.f, cg::bound(-world_light_dir.z, 0.f, 1.f));
#endif

        lights->AddLight(avScene::/*GlobalInfluence*/LocalInfluence, avScene::ConicalLight,
            position, direction, light.distanceFalloff, spotFalloff, 
            light.color, 0.60f, 0.35f);
    }


}


} // namespace avScene
