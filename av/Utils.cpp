#include "stdafx.h"


#include "Utils.h"

#include <osgUtil/Optimizer>
#include <osgUtil/CullVisitor>
#include <osgUtil/IntersectionVisitor>
#include <osgUtil/PlaneIntersector>

using namespace utils;

#undef foreach

#define foreach(Iter, Container)\
    for ( auto Iter = Container.begin(); Iter != Container.end(); ++Iter )

//////////////////////////////////////////////////////////////////////////
static bool s_bMouseShowed = true;
bool utils::GetMouseCursorShowState()
{
    return s_bMouseShowed;
}

//void utils::ShowMouseCursor( bool bShow )
//{
//    if ( bShow != s_bMouseShowed )
//    {
//        ShowCursor( bShow ? TRUE : FALSE );
//        SetCursor( bShow ? LoadCursor( NULL, IDC_ARROW ) : NULL );
//        s_bMouseShowed = bShow;
//    }
//}


/////////////////////////////////////////////////////////////////////

// helpful vector absolution
static __forceinline void _vectorAbs( osg::Vec3f & vVec )
{
    DWORD * pPtr = reinterpret_cast<DWORD *>(&vVec);
    *(pPtr++) &= 0x7FFFFFFF;
    *(pPtr++) &= 0x7FFFFFFF;
    *(pPtr++) &= 0x7FFFFFFF;
    return;
}

// helpful vector clamping
static __forceinline void _vectorZeroClamp( osg::Vec3f & vVec )
{
    DWORD * pPtr = reinterpret_cast<DWORD *>(&vVec);
    DWORD & val0 = *(pPtr++);
    if (val0 & 0x80000000)
        val0 ^= val0;
    DWORD & val1 = *(pPtr++);
    if (val1 & 0x80000000)
        val1 ^= val1;
    DWORD & val2 = *(pPtr++);
    if (val2 & 0x80000000)
        val2 ^= val2;
    return;
}

// distance to axis-aligned bounding box
float utils::GetDistanceToAABBSqr( const osg::Vec3f & vPos, const osg::BoundingBox & aabb )
{
    const osg::Vec3f vBoxCenter = aabb.center();
    const osg::Vec3f vBoxHalf = aabb._max - vBoxCenter;

    osg::Vec3f vDelta = vPos - vBoxCenter;
    _vectorAbs(vDelta);
    vDelta -= vBoxHalf;
    _vectorZeroClamp(vDelta);

    return vDelta * vDelta;
}



/////////////////////////////////////////////////////////////////////
void utils::DisableCulling( osg::Node* pNode )
{
    pNode->setCullingActive( false );
    if ( pNode->asGroup() )
    {
        for( uint32_t i = 0; i < pNode->asGroup()->getNumChildren(); i++ )
        {
            DisableCulling( pNode->asGroup()->getChild(i) );
        }
    }
}

//////////////////////////////////////////////////////////////////////////
struct postProcRecData
{
    unsigned nCurLevel;

    osg::ref_ptr<osg::StateAttribute> pTex0;
    osg::ref_ptr<osg::StateAttribute> pTex1;
    osg::ref_ptr<osg::StateAttribute> pTex2;

    osg::ref_ptr<osg::Vec3Array> pNormalUp;

    postProcRecData()
        : nCurLevel(0)
    {
        return;
    }
};

static void _postprocessModelInternal(osg::Node * pNode, const postProcRecData & curRecData)
{
    avAssert(pNode);

    postProcRecData newRecData = curRecData;
    newRecData.nCurLevel++;

    // clear state set, leaving only 0,1,2 textures active
    osg::StateSet* pStateSet = pNode->getStateSet();
    if (pStateSet)
    {
        osg::ref_ptr<osg::StateAttribute> m_pAttr0, m_pAttr1, m_pAttr2;
        m_pAttr0 = pStateSet->getTextureAttribute(0, osg::StateAttribute::TEXTURE);
        m_pAttr1 = pStateSet->getTextureAttribute(1, osg::StateAttribute::TEXTURE);
        m_pAttr2 = pStateSet->getTextureAttribute(2, osg::StateAttribute::TEXTURE);

        osg::ref_ptr<osg::StateAttribute> m_pPolyOffset;
        m_pPolyOffset = pStateSet->getAttribute(osg::StateAttribute::POLYGONOFFSET);

        pStateSet->clear();
        bool bEmptySS = true;
        if (m_pAttr0.valid() && (m_pAttr0 != newRecData.pTex0))
        {
            pStateSet->setTextureAttribute(0, m_pAttr0.get());
            newRecData.pTex0 = m_pAttr0;
            bEmptySS = false;
        }
        if (m_pAttr1.valid() && (m_pAttr1 != newRecData.pTex1))
        {
            pStateSet->setTextureAttribute(1, m_pAttr1.get());
            newRecData.pTex1 = m_pAttr1;
            bEmptySS = false;
        }
        if (m_pAttr2.valid() && (m_pAttr2 != newRecData.pTex2))
        {
            pStateSet->setTextureAttribute(2, m_pAttr2.get());
            newRecData.pTex2 = m_pAttr2;
            bEmptySS = false;
        }

        if (m_pPolyOffset.valid())
        {
            pStateSet->setAttributeAndModes(m_pPolyOffset.get(), osg::StateAttribute::ON);
            bEmptySS = false;
        }

        pStateSet->setNestRenderBins(false);

        if (bEmptySS)
            pNode->setStateSet(NULL);
    }

    // xEvguenZ: while we have driver problems - blended node is commented
#if 0
    // Add alpha blending state-set
    // To be removed in future I suppose...
    if (pNode->getName() == "AlphaBlended")
    {
        osg::StateSet * pStateSet = pNode->getOrCreateStateSet();
        // set proper blending
        pStateSet->setAttributeAndModes(new osg::BlendFunc(osg::BlendFunc::SRC_ALPHA, osg::BlendFunc::ONE_MINUS_SRC_ALPHA), osg::StateAttribute::ON);
        // set renderbin correct order
        pStateSet->setNestRenderBins(false);
        pStateSet->setRenderBinDetails(RENDER_BIN_BLENDED_MODELS_OUTSIDE, "DepthSortedBin");
        // disable alpha-to-coverage for that node
        pStateSet->setMode(0x809E/*GL_SAMPLE_ALPHA_TO_COVERAGE_ARB*/, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
    }
#endif

    // Fix empty normals bug
    if (pNode->asGeode())
    {
        osg::Geode * pGeode = pNode->asGeode();
        for (unsigned i = 0; i < pGeode->getNumDrawables(); ++i)
        {
            osg::Geometry * pGeom = pGeode->getDrawable(i)->asGeometry();
            if (pGeom && pGeom->getNormalBinding() == osg::Geometry::BIND_OFF)
            {
                pGeom->setNormalBinding(osg::Geometry::BIND_OVERALL);
                pGeom->setNormalArray(curRecData.pNormalUp.get());
            }
        }
    }

    // Process children
    if (pNode->asGroup())
    {
        for (unsigned i = 0; i < pNode->asGroup()->getNumChildren(); ++i)
            _postprocessModelInternal(pNode->asGroup()->getChild(i), newRecData);
    }
}

void utils::PostprocessModel( osg::Node * pNode )
{
    avAssert( pNode );

    postProcRecData recData;
    osg::Vec3f vUpNormal = osg::Z_AXIS;
    recData.pNormalUp = new osg::Vec3Array(1, &vUpNormal);
    recData.pNormalUp->setDataVariance(osg::Object::STATIC);

    _postprocessModelInternal(pNode, recData);
}

//////////////////////////////////////////////////////////////////////////
void utils::SetDynamicNode( osg::Node* pNode, bool bDynamicChildren /*= true*/ )
{
    avAssert( pNode );
    pNode->setDataVariance( osg::Object::DYNAMIC );

    if ( bDynamicChildren )
    {
        osg::Group* pGroup = pNode->asGroup();
        if ( pGroup )
        {
            for( uint32_t nIndex = 0; nIndex < pGroup->getNumChildren(); nIndex++ )
            {
                SetDynamicNode( pGroup->getChild( nIndex ), bDynamicChildren );
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////
void utils::RemoveNodeFromAllParents( osg::Node* pNode )
{
    avAssert( pNode );

    osg::Node::ParentList& cParents = pNode->getParents();
    for ( uint32_t nIndex = 0; nIndex < cParents.size(); nIndex++ )
    {
        cParents[ nIndex ]->removeChild( pNode );
    }
}

//////////////////////////////////////////////////////////////////////////
bool utils::SetTextureUniform( const char* szTextureName, const char* szUniformName, int nID, osg::StateSet* pStateSet,
                               osg::Texture::WrapMode eWrapMode /*= osg::Texture::REPEAT*/, const osgDB::Options* options )
{
    avAssert( szTextureName && szUniformName && nID >= 0 && pStateSet );

    osg::Texture2D* pTexture = new osg::Texture2D(osgDB::readImageFile(szTextureName, options));// GetDatabase()->LoadTexture( szTextureName, eWrapMode );

    if ( pTexture == NULL )
        return false;

    pTexture->setWrap(  osg::Texture::WRAP_S, eWrapMode );
    pTexture->setWrap(  osg::Texture::WRAP_T, eWrapMode );
    pTexture->setWrap(  osg::Texture::WRAP_R, eWrapMode );

    pStateSet->addUniform( new osg::Uniform( szUniformName, nID ) );
    pStateSet->setTextureAttribute( nID, pTexture );
    return true;
}

//////////////////////////////////////////////////////////////////////////
float utils::GetScreenClarity( osgUtil::CullVisitor * pCV )
{
    // get screen pixel clarity
    const osg::Matrix & P = *pCV->getProjectionMatrix();
    const osg::Viewport & W = *pCV->getViewport();

    // scaling for horizontal pixels
    float P00 = P(0,0) * W.width() * 0.5f;
    float P20_00 = P(2,0)* W.width() * 0.5f + P(2,3) * W.width() * 0.5f;
    osg::Vec3 scale_00(P00, 0.f, P20_00);

    // scaling for vertical pixels
    float P10 = P(1,1) * W.height() * 0.5f;
    float P20_10 = P(2,1) * W.height() * 0.5f + P(2,3) * W.height() * 0.5f;
    osg::Vec3 scale_10(0.f, P10, P20_10);

    // gotcha!
    return fabsf(P(2,3) * 0.7071067811f / sqrtf(scale_00.length2() + scale_10.length2()));
}

//////////////////////////////////////////////////////////////////////////
bool utils::IsPowerOf2( unsigned int nValue )
{
    // More examples how to test if number is power of 2 you can find on:
    // http://www.acetheinterview.com/cgi-bin/answers.cgi?action=answers&number=4&topic=000007.ubb&q_id=7
    return ( ( nValue & ( nValue - 1 ) ) == 0 );
}

//////////////////////////////////////////////////////////////////////////
bool utils::CheckIfAFileExists( const std::string& cAbsolutePath )
{
    return ( GetFileAttributes( cAbsolutePath.c_str() ) != INVALID_FILE_ATTRIBUTES );
}

/////////////////////////////////////////////////////////////////////
//						NodeFinder
/////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
NodeFinder::NodeFinder() :
    _pNode( NULL ),
    _pResultNode( NULL )
{
}

//////////////////////////////////////////////////////////////////////////
NodeFinder::~NodeFinder()
{
}

//////////////////////////////////////////////////////////////////////////
void NodeFinder::FindByName(osg::Node* pNode, const char* szName)
{
    avAssert( pNode && szName );

    const char* szNodeName = pNode->getName().c_str();
    if (szNodeName && !strcmp(szNodeName, szName))
    {
        _pResultNode = pNode;
    } else
    if (pNode->asGroup())
    {
        for (uint32_t i = 0; i < pNode->asGroup()->getNumChildren(); i++)
        {
            FindByName(pNode->asGroup()->getChild(i), szName);
        }
    }
}

//////////////////////////////////////////////////////////////////////////
void NodeFinder::FindByName_nocase(osg::Node* pNode, const char* szName)
{
    avAssert( pNode && szName );

    const char* szNodeName = pNode->getName().c_str();
    if (szNodeName && !_stricmp(szNodeName, szName))
    {
        _pResultNode = pNode;
    }
    else if (pNode->asGroup())
    {
        for (uint32_t i = 0; i < pNode->asGroup()->getNumChildren(); i++)
        {
            FindByName_nocase(pNode->asGroup()->getChild(i), szName);
        }
    }
}

//////////////////////////////////////////////////////////////////////////
osg::Node* NodeFinder::FindChildByName(const char* szName)
{
    _pResultNode = NULL;
    if (szName && _pNode)
        FindByName(_pNode, szName);
    return _pResultNode;
}

//////////////////////////////////////////////////////////////////////////
osg::Node* NodeFinder::FindChildByName_nocase(const char* szName)
{
    _pResultNode = NULL;
    if (szName && _pNode)
        FindByName_nocase(_pNode, szName);
    return _pResultNode;
}

/////////////////////////////////////////////////////////////////////
//                      Tree statistics
/////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
TreeStatGathering::TreeStatGathering()
     : osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN)
{
}

void TreeStatGathering::apply( osg::Node & node )
{
    ++m_statTreeData.nGeneralNodes;
    checkStateSet(node.getStateSet());
    traverse(node);
}

void TreeStatGathering::apply( osg::Group & node )
{
    ++m_statTreeData.nGroups;
    checkStateSet(node.getStateSet());
    traverse(node);
}
void TreeStatGathering::apply( osg::Transform & node )
{
    ++m_statTreeData.nTransforms;
    checkStateSet(node.getStateSet());
    traverse(node);
}
void TreeStatGathering::apply( osg::LOD & node )
{
    ++m_statTreeData.nLODs;
    checkStateSet(node.getStateSet());
    traverse(node);
}

void TreeStatGathering::apply( osg::Geode & node )
{
    ++m_statTreeData.nGeodes;
    checkStateSet(node.getStateSet());
    checkGeode(&node);
    traverse(node);
}

// check state set
void TreeStatGathering::checkStateSet( const osg::StateSet * pSS )
{
    if (pSS)
    {
        // handle
        ++m_statTreeData.nStateSets;
        m_uniqueStateSets.insert(pSS);

        // check textures
        const osg::StateAttribute * pAttr0, * pAttr1, * pAttr2;
        pAttr0 = pSS->getTextureAttribute(0, osg::StateAttribute::TEXTURE);
        pAttr1 = pSS->getTextureAttribute(1, osg::StateAttribute::TEXTURE);
        pAttr2 = pSS->getTextureAttribute(2, osg::StateAttribute::TEXTURE);
        if (pAttr0)
            m_uniqueTextures.insert(pAttr0->asTexture());
        if (pAttr1)
            m_uniqueTextures.insert(pAttr1->asTexture());
        if (pAttr2)
            m_uniqueTextures.insert(pAttr2->asTexture());
    }
}

// check geode
void TreeStatGathering::checkGeode( const osg::Geode * pGeode )
{
    // check every drawable
    for (unsigned i = 0; i < pGeode->getNumDrawables(); ++i)
    {
        // handle state set
        const osg::Drawable * pDrawable = pGeode->getDrawable(i);
        checkStateSet(pDrawable->getStateSet());
        // handle geometry
        const osg::Geometry * pGeometry = pDrawable->asGeometry();
        ++m_statTreeData.nDrawables;
        // vertices
        m_statTreeData.nVertices += pGeometry->getVertexArray()->getNumElements();

        // check all DIPs
        const osg::Geometry::PrimitiveSetList & primitives = pGeometry->getPrimitiveSetList();
        for (unsigned i = 0; i < primitives.size(); ++i)
        {
            // one more DIP
            ++m_statTreeData.nDIPCalls;
            // elements being rendered
            m_statTreeData.nElements += primitives[i]->getNumPrimitives();
        }
    }
}

//////////////////////////////////////////////////////////////////////////
//                        AABB calculation
//////////////////////////////////////////////////////////////////////////

//
// Visitor, which calculates result world AABB for model
//

class AABBCalculationVisitor : public osg::NodeVisitor
{

public:

    AABBCalculationVisitor()
        : NodeVisitor(osg::NodeVisitor::TRAVERSE_ACTIVE_CHILDREN)
    {
        return;
    }

    // get collected box
    const osg::BoundingBox & getCollectedAABB() const { return m_bbCollectedBox; }

private:

    // primitive functor
    struct ProcessDrawableAABB : public osg::PrimitiveFunctor
    {
        ProcessDrawableAABB(osg::Matrix * pLocalTrans)
            : _pLocalTrans(pLocalTrans)
        {
            _vertices2f = NULL;
            _vertices3f = NULL;
            _vertices4f = NULL;
            _vertices2d = NULL;
            _vertices3d = NULL;
            _vertices4d = NULL;
        }

        virtual void setVertexArray(unsigned int,const osg::Vec2* vertices) { _vertices2f = vertices; }
        virtual void setVertexArray(unsigned int,const osg::Vec3* vertices) { _vertices3f = vertices; }
        virtual void setVertexArray(unsigned int,const osg::Vec4* vertices) { _vertices4f = vertices; }
        virtual void setVertexArray(unsigned int,const osg::Vec2d* vertices) { _vertices2d = vertices; }
        virtual void setVertexArray(unsigned int,const osg::Vec3d* vertices) { _vertices3d = vertices; }
        virtual void setVertexArray(unsigned int,const osg::Vec4d* vertices) { _vertices4d = vertices; }

        template<typename T>
        void _drawArrays(T* vert, T* end)
        {
            for(;vert<end;++vert)
            {
                vertex(*vert);
            }
        }
        template<typename T, typename I>
        void _drawElements(T* vert, I* indices, I* end)
        {
            for(;indices<end;++indices)
            {
                vertex(vert[*indices]);
            }
        }

        virtual void drawArrays(GLenum,GLint first,GLsizei count)
        {
            if      (_vertices3f) _drawArrays(_vertices3f+first, _vertices3f+(first+count));
            else if (_vertices2f) _drawArrays(_vertices2f+first, _vertices2f+(first+count));
        }
        virtual void drawElements(GLenum,GLsizei count,const GLubyte* indices)
        {
            if      (_vertices3f) _drawElements(_vertices3f, indices, indices + count);
            else if (_vertices2f) _drawElements(_vertices2f, indices, indices + count);
        }
        virtual void drawElements(GLenum,GLsizei count,const GLushort* indices)
        {
            if      (_vertices3f) _drawElements(_vertices3f, indices, indices + count);
            else if (_vertices2f) _drawElements(_vertices2f, indices, indices + count);
        }
        virtual void drawElements(GLenum,GLsizei count,const GLuint* indices)
        {
            if      (_vertices3f) _drawElements(_vertices3f, indices, indices + count);
            else if (_vertices2f) _drawElements(_vertices2f, indices, indices + count);
        }

        virtual void begin(GLenum) {}
        virtual void vertex(const osg::Vec2& vert) { _bb.expandBy(apply_cs(osg::Vec3(vert[0],vert[1],0.0f))); }
        virtual void vertex(const osg::Vec3& vert) { _bb.expandBy(apply_cs(vert)); }
        virtual void vertex(const osg::Vec4& vert) { if (vert[3]!=0.0f) _bb.expandBy(apply_cs(osg::Vec3(vert[0],vert[1],vert[2])/vert[3])); }
        virtual void vertex(const osg::Vec2d& vert) { _bb.expandBy(apply_cs(osg::Vec3(vert[0],vert[1],0.0f))); }
        virtual void vertex(const osg::Vec3d& vert) { _bb.expandBy(apply_cs(vert)); }
        virtual void vertex(const osg::Vec4d& vert) { if (vert[3]!=0.0f) _bb.expandBy(apply_cs(osg::Vec3(vert[0],vert[1],vert[2])/vert[3])); }
        virtual void vertex(float x,float y)  { _bb.expandBy(apply_cs(osg::Vec3(x,y,0.0f))); }
        virtual void vertex(float x,float y,float z) { _bb.expandBy(apply_cs(osg::Vec3(x,y,z))); }
        virtual void vertex(float x,float y,float z,float w) { if (w!=0.0f) _bb.expandBy(apply_cs(osg::Vec3(x,y,z)/w)); }
        virtual void vertex(double x,double y)  { _bb.expandBy(apply_cs(osg::Vec3(x,y,0.0f))); }
        virtual void vertex(double x,double y,double z) { _bb.expandBy(apply_cs(osg::Vec3(x,y,z))); }
        virtual void vertex(double x,double y,double z,double w) { if (w!=0.0f) _bb.expandBy(apply_cs(osg::Vec3(x,y,z)/w)); }
        virtual void end() {}

        osg::Vec3 apply_cs(const osg::Vec3 &pnt) { return (_pLocalTrans) ? (pnt * (*_pLocalTrans)) : pnt; }

        const osg::Matrix * _pLocalTrans;

        const osg::Vec2*     _vertices2f;
        const osg::Vec3*     _vertices3f;
        const osg::Vec4*     _vertices4f;
        const osg::Vec2d*    _vertices2d;
        const osg::Vec3d*    _vertices3d;
        const osg::Vec4d*    _vertices4d;

        osg::BoundingBox     _bb; 
    };

    // model matrix working
    void pushModelMatrix( osg::RefMatrix * matrix ) { m_aModelStack.push_back(matrix); }
    void popModelMatrix() { m_aModelStack.pop_back(); } 
    osg::RefMatrix * getModelMatrix() { return m_aModelStack.empty() ? NULL :  m_aModelStack.back().get(); }
    const osg::RefMatrix * getModelMatrix() const { return m_aModelStack.empty() ? NULL :  m_aModelStack.back().get(); }

    // apply for common node
    virtual void apply(osg::Node& node)
    {
        traverse(node);
    }
    // geode apply
    virtual void apply(osg::Geode& geode)
    {
        for (size_t i = 0; i < geode.getNumDrawables(); ++i)
        {
            ProcessDrawableAABB aabb_calc(getModelMatrix());
            geode.getDrawable(i)->accept(aabb_calc);
            m_bbCollectedBox.expandBy(aabb_calc._bb);
        }
    }
    // transform apply
    virtual void apply(osg::Transform& transform)
    {
        osg::ref_ptr<osg::RefMatrix> matrix = m_aModelStack.empty() ?
            new osg::RefMatrix() :
            new osg::RefMatrix(*m_aModelStack.back());
        transform.computeLocalToWorldMatrix(*matrix, this);

        pushModelMatrix(matrix.get());
        traverse(transform);
        popModelMatrix();
    }

private:

    typedef std::vector< osg::ref_ptr<osg::RefMatrix> > MatrixStack;
    MatrixStack m_aModelStack;

private:

    osg::BoundingBox m_bbCollectedBox;
};

//////////////////////////////////////////////////////////////////////////
osg::BoundingBox utils::CalculateModelAABB( osg::Node * pNode )
{
    AABBCalculationVisitor aabb_visitor;
    pNode->accept(aabb_visitor);
    return aabb_visitor.getCollectedAABB();
}


//////////////////////////////////////////////////////////////////////////
float utils::ConvertTraineeDensityToExpSceneFog( float fTraineeDensity, float * fVisDistPtr )
{
    // calculate real visibility distance somehow
    static const double dLN256 = log(256.0);
    const float
        fDistFactor = powf(1.f - fTraineeDensity, 2.75f),
        fRealVisDist = cg::lerp01( 125.0f, 40000.f, fDistFactor);
    // save vis dist
    if (fVisDistPtr)
        *fVisDistPtr = fRealVisDist;
    // calculate fogging exponent
    return float(dLN256 / double(fRealVisDist));
}

//////////////////////////////////////////////////////////////////////////
float utils::ConvertTraineeDensityToExpReflFog( float fTraineeDensity, float * fVisDistPtr )
{
    // calculate real visibility distance somehow
    static const double dLN256 = log(256.0);
    const float
        fDistFactor = powf(1.f - fTraineeDensity, 3.5f),
        fRealVisDist = cg::lerp01( 40.0f, 10000.f, fDistFactor);
    // save vis dist
    if (fVisDistPtr)
        *fVisDistPtr = fRealVisDist;
    // calculate fogging exponent
    return float(dLN256 / double(fRealVisDist));
}

float utils::GetHorizontalIntersectionRadius( osg::Node* pNode, float fHeight )
{
    float Radius ( 0.0f );

    if (pNode == NULL)
    {
        return Radius;
    }

    osg::Plane plane(osg::Vec3f ( 0.0f, 0.0f, 1.0f ), -fHeight);
    osg::Polytope boundingPolytope;
    osg::ref_ptr<osgUtil::PlaneIntersector> intersector = new osgUtil::PlaneIntersector(plane, boundingPolytope);
    osgUtil::IntersectionVisitor _intersectionVisitor;

    _intersectionVisitor.reset();
    _intersectionVisitor.setTraversalMask(0xFFFFFFFF);
    _intersectionVisitor.setIntersector( intersector.get() );

    pNode->accept(_intersectionVisitor);

    osgUtil::PlaneIntersector::Intersections& intersections = intersector->getIntersections();

    // Assume symmetric form (used for buoys mostly)

    foreach(itr, intersections)
    {
        osgUtil::PlaneIntersector::Intersection& intersection = *itr;
        foreach(pitr, intersection.polyline)
        {
            osg::Vec3d V (*pitr);
            if (intersection.matrix.valid())
            {
                V = V * (*intersection.matrix);
            }
            V.z() = 0.0;
            float Length = V.length();
            if (Length > Radius)
            {
                Radius = Length;
            }
        }
    }

    return Radius;
}


namespace utils
{
    bool replace(std::string& str, const std::string& from, const std::string& to) {
        size_t start_pos = str.find(from);
        if(start_pos == std::string::npos)
            return false;
        str.replace(start_pos, from.length(), to);
        return true;
    }

    void replaceAll(std::string& str, const std::string& from, const std::string& to) {
        if(from.empty())
            return;
        size_t start_pos = 0;
        while((start_pos = str.find(from, start_pos)) != std::string::npos) {
            str.replace(start_pos, from.length(), to);
            start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
        }
    }

    std::string format( const char * str )
    {
        std::string source(str);
        replaceAll(source,std::string("$define"), std::string("\n#define"));
        replaceAll(source,std::string("$if"), std::string("\n#if"));
        replaceAll(source,std::string("$else"), std::string("\n#else"));
        replaceAll(source,std::string("$endif"), std::string("\n#endif"));
        replaceAll(source,std::string("$extention"), std::string("\n#extention"));
        replaceAll(source,std::string("$"), std::string("\n "));
        return source;
    }

    std::string format( std::string const & str )
    {
        return format(str.c_str());
    }
}
