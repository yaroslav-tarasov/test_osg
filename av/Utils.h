#pragma once

#include "utils/random.h"


//
// Module namespace
//

namespace utils
{
    void        ShowMouseCursor( bool bShow );
    bool        GetMouseCursorShowState();
    float       GetDistanceToAABBSqr( const osg::Vec3f & vPos, const osg::BoundingBox & aabb );
    void        DisableCulling( osg::Node* pNode );
    void        PostprocessModel( osg::Node* pNode );
    void        SetDynamicNode( osg::Node* pNode, bool bDynamicChildren = true );
    void        RemoveNodeFromAllParents( osg::Node* pNode );
    bool        SetTextureUniform( const char* szTextureName, const char* szUniformName, int nID, osg::StateSet* pStateSet, osg::Texture::WrapMode eWrapMode = osg::Texture::REPEAT, const osgDB::Options* options = nullptr);
    float       GetScreenClarity( osgUtil::CullVisitor * pCV );
    bool        IsPowerOf2( unsigned int nValue );
    bool        CheckIfAFileExists( const std::string& cAbsolutePath );

    osg::BoundingBox CalculateModelAABB( osg::Node * pNode );

    float ConvertTraineeDensityToExpSceneFog( float fTraineeDensity, float * fVisDistPtr = NULL );
    float ConvertTraineeDensityToExpReflFog( float fTraineeDensity, float * fVisDistPtr = NULL );
    float GetHorizontalIntersectionRadius( osg::Node* pNode, float fHeight );

    // get unit vector of proper direction
    __forceinline cg::point_3f celestialUnitVector( float fAzimuth, float fElevation )
    {
        float sin_phi, cos_phi, sin_theta, cos_theta;
        cg::fsincos(fAzimuth, &sin_phi, &cos_phi);
        cg::fsincos(fElevation, &sin_theta, &cos_theta);
        return cg::point_3f(sin_phi * cos_theta, cos_phi * cos_theta, sin_theta);
    }

/////////////////////////////////////////////////////////////////////
//                  CatmullRomSpline
/////////////////////////////////////////////////////////////////////
template <class T>
class CatmullRomSpline
{
    T                   _c0, _c1, _c2, _c3;

public:
    CatmullRomSpline( const T& v0, const T& v1, const T& v2, const T& v3, double dbTension = 1.0 )
    {
        // Modify first and last control points to avoid errors
        // Spline interpolation doesn't work properly if distances between points are different
        double dbLength = (v2 - v1).length() * dbTension;

        T _v0 = (v1 - v0);  
        _v0.normalize();
        _v0 = v1 - _v0*dbLength;

        T _v3 = (v3 - v2);
        _v3.normalize();
        _v3 = v2 + _v3*dbLength;

        // Compute control data
        _c0 =             v1;
        _c1 =  -_v0*0.5          + v2*0.5;
        _c2 =   _v0     - v1*2.5 + v2*2.0 - _v3*0.5;
        _c3 =  -_v0*0.5 + v1*1.5 - v2*1.5 + _v3*0.5;
    }

    T Interpolate( float t )
    {
        return ( ( ( _c3 * t + _c2 ) * t +_c1 ) * t + _c0 );
    }
};

/////////////////////////////////////////////////////////////////////
//						NodeFinder
/////////////////////////////////////////////////////////////////////
class NodeFinder
{
private:
	osg::Node*			_pNode;
	osg::Node*			_pResultNode;

    void                FindByName(osg::Node* pNode, const char* szName);
    void                FindByName_nocase(osg::Node* pNode, const char* szName);
public:
						NodeFinder();
						~NodeFinder();

	inline void			SetNode(osg::Node* pNode) { _pNode = pNode; }
	inline osg::Node*	GetNode() { return _pNode; }

    osg::Node*          FindChildByName(const char* szName);
    osg::Node*          FindChildByName_nocase(const char* szName);
};

/////////////////////////////////////////////////////////////////////
//                      Tree statistics
/////////////////////////////////////////////////////////////////////

class TreeStatGathering : public osg::NodeVisitor
{
public:

    // constructor
    TreeStatGathering();

private:

    // statistics structure
    struct TreeStatData
    {
        unsigned nGeneralNodes;
        unsigned nStateSets;
        unsigned nGroups, nTransforms, nLODs, nGeodes;
        unsigned nDrawables, nDIPCalls, nBuffers, nVertices, nElements;

        TreeStatData() { memset(this, 0, sizeof(TreeStatData)); }
    } m_statTreeData;

    // virtual members
    void apply( osg::Node & node );
    void apply( osg::Group & node );
    void apply( osg::Transform & node );
    void apply( osg::LOD & node );
    void apply( osg::Geode & node );

private:

    // check state set
    void checkStateSet( const osg::StateSet * pSS );

    // check geode
    void checkGeode( const osg::Geode * pGeode );

private:

    std::set<const osg::StateSet *> m_uniqueStateSets;
    std::set<const osg::Texture *> m_uniqueTextures;
    //std::set<const osg::Texture *> m_uniqueVBOs;

};

/////////////////////////////////////////////////////////////////////
//                          NodeCallback
/////////////////////////////////////////////////////////////////////
template <class T>
class NodeCallback : public osg::NodeCallback
{
public:

    NodeCallback( T * object, void (T::*func)( osg::NodeVisitor * nv ), bool isPure = false )
        : _object(object)
        , _func(func)
        , _isPureCallback(isPure)
    {
    }

    NodeCallback( const NodeCallback & other, const osg::CopyOp & copyop = osg::CopyOp::SHALLOW_COPY )
        : osg::NodeCallback(other, copyop)
        , _object(other._object)
        , _func(other._func)
        , _isPureCallback(other._isPureCallback)
    {
    }

    virtual void operator()( osg::Node * node, osg::NodeVisitor * nv )
    {
        (_object->*_func)(nv);

        if (!_isPureCallback)
            osg::NodeCallback::operator()(_object, nv);
    }

private:

    T * _object;
    void (T::*_func)( osg::NodeVisitor * nv );
    bool _isPureCallback;
};

template<class T>
inline NodeCallback<T> * makeNodeCallback( T * object, void (T::*func)( osg::NodeVisitor * nv ), bool isPure = false )
{
	return new NodeCallback<T>(object, func, isPure);
}

/////////////////////////////////////////////////////////////////////
//              NodeBoundSubstitution
/////////////////////////////////////////////////////////////////////

class NodeBoundSubstitution : public osg::Drawable::ComputeBoundingBoxCallback
{
public:

    NodeBoundSubstitution( const osg::BoundingBox & bb )
        : refBB(bb)
    {
    }

private:

    osg::BoundingBox computeBound( const osg::Drawable & /*drawable*/) const
    {
        return refBB;
    }

    const osg::BoundingBox & refBB;
};

} // namespace utils

template<class T, class U> T safe_cast(U p)
{
#ifdef _DEBUG
	avAssertMessage( dynamic_cast<T> (p) == static_cast<T> (p) , "Failed to cast pointer" );
#endif
	return static_cast<T> (p);
} 

#define svReleaseMem( val ) if ( val != NULL ) { delete val; val = NULL; }
#define svReleaseMemBuffer( val ) if ( val != NULL ) { delete[] val; val = NULL; }

namespace utils
{
    bool replace(std::string& str, const std::string& from, const std::string& to);
    void replaceAll(std::string& str, const std::string& from, const std::string& to);
    std::string format( const char * str );
    std::string format( std::string const & str );
}
