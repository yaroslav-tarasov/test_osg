#include "stdafx.h"

#include "av/avLine/Line.h"
#include "av/avCore/avCore.h"
#include "av/avLine/LineManager.h"
#include "av/avScene/Scene.h"

using namespace avLine;

const double DEFAULT_SEGMENT_LENGTH = 0.15;

#define DEFAULT_NODE_MASK 0x01	                // No water reflection

//////////////////////////////////////////////////////////////////////////
static inline osg::Vec3d LookAt(osg::Matrixd& m, const osg::Vec3d& pos, const osg::Vec3d& at, const osg::Vec3d& up, bool firstTime, osg::Vec3d & prevDirStraight, float & fRadRatio )
{
    osg::Vec3d vCurDir = at - pos;
    vCurDir.normalize();

    osg::Vec3d dirN;
    if (!firstTime)
    {
        // get direction by evening the bisector
        dirN = vCurDir + prevDirStraight;
        dirN.normalize();
        // save for next step
        prevDirStraight = vCurDir;
        // get radius ratio for correct widening
        fRadRatio = dirN * prevDirStraight;
    }
    else
    {
        dirN = vCurDir;
        prevDirStraight = vCurDir;
        fRadRatio = 1.0f;
    }

    // left vector
    osg::Vec3d lftN = dirN ^ up;
    lftN.normalize();

    // up vector
    osg::Vec3d upN = lftN ^ dirN;

    // fill matrix
    double * pMatrixData = m.ptr();
    pMatrixData[ 0] = lftN.x();
    pMatrixData[ 1] = lftN.y();
    pMatrixData[ 2] = lftN.z();
    pMatrixData[ 3] = 0.0;
    pMatrixData[ 4] = dirN.x();
    pMatrixData[ 5] = dirN.y();
    pMatrixData[ 6] = dirN.z();
    pMatrixData[ 7] = 0.0;
    pMatrixData[ 8] = upN.x();
    pMatrixData[ 9] = upN.y();
    pMatrixData[10] = upN.z();
    pMatrixData[11] = 0.0;
    pMatrixData[12] = pos.x();
    pMatrixData[13] = pos.y();
    pMatrixData[14] = pos.z();
    pMatrixData[15] = 1.0;

    // return up-vector
    return upN;
}

//////////////////////////////////////////////////////////////////////////
static void MakeLastBoneMatrix(osg::Matrixd & m, const osg::Vec3d & pos, const osg::Vec3d& up, osg::Vec3d & dir )
{
    osg::Vec3d dirN;
    dirN = dir;

    // left vector
    osg::Vec3d lftN = dirN ^ up;
    lftN.normalize();

    // up vector
    osg::Vec3d upN = lftN ^ dirN;

    // fill matrix
    double * pMatrixData = m.ptr();
    pMatrixData[ 0] = lftN.x();
    pMatrixData[ 1] = lftN.y();
    pMatrixData[ 2] = lftN.z();
    pMatrixData[ 3] = 0.0;
    pMatrixData[ 4] = dirN.x();
    pMatrixData[ 5] = dirN.y();
    pMatrixData[ 6] = dirN.z();
    pMatrixData[ 7] = 0.0;
    pMatrixData[ 8] = upN.x();
    pMatrixData[ 9] = upN.y();
    pMatrixData[10] = upN.z();
    pMatrixData[11] = 0.0;
    pMatrixData[12] = pos.x();
    pMatrixData[13] = pos.y();
    pMatrixData[14] = pos.z();
    pMatrixData[15] = 1.0;
}


//////////////////////////////////////////////////////////////////////////
Line::Line( uint32_t nID ) : 
    _bShowLine( true ),
    _dbRadius( 1.0 ),
    _geode(new osg::Geode()),
    _dbSegmentLength( DEFAULT_SEGMENT_LENGTH ),
    _dbSegmentExtraOffset( 0.0 ),
    _cRadiusUniformPtr( new osg::Uniform(osg::Uniform::FLOAT, "radius" ) ),
    _cTintingColorUniformPtr( new osg::Uniform( osg::Uniform::FLOAT_VEC3, "vDiffuseColor" ) ),
    m_bUseSplines(true),
    m_bNeedToUpdateDrumLine( false ),
    m_fTexLenStart(0.0f), 
    m_fTexLenEnd(0.0f),
    m_nStartChangeFrom( 0 ),
    m_nStartSegmentPointsChangeFrom( 0 ),
    m_nBonesMapWidth ( 384 ), 
    m_nBonesMapHeight ( 1 )
{

    // Set node mask to not reflect lines 
    setNodeMask( DEFAULT_NODE_MASK );

    // Create bounding box callback
    _cBoundingBoxCallbackPtr = new CBoundingBoxCallback( _cBoundingBox ); 

    addChild(_geode.get());

    // Setup uniforms
    osg::StateSet* pStateSet = _geode->getOrCreateStateSet();
    pStateSet->setNestRenderBins(false);
    _cRadiusUniformPtr->set( (float)_dbRadius );
    _cTintingColorUniformPtr->set( osg::Vec3( 1,1,1 ) );
    pStateSet->addUniform( _cRadiusUniformPtr.get() );
    pStateSet->addUniform( _cTintingColorUniformPtr.get() );

    const int BonesMapTextureUnit = 1;
    pStateSet->addUniform( new osg::Uniform("BonesMapTexture", BonesMapTextureUnit) );
    m_BonesMapTexture = createBonesMap();
    m_BonesMapTexture->setName("LineBonesMapTexture");
    pStateSet->setTextureAttribute(BonesMapTextureUnit, m_BonesMapTexture.get());

}

//////////////////////////////////////////////////////////////////////////
Line::~Line()
{
}



osg::Drawable* AddText ( const char* Text, const osg::Vec3& pos,  const osg::Vec4f& Color )
{
    osgText::Text *text1 = new osgText::Text;
    text1->setFont("fonts/arial.ttf");
    text1->setCharacterSize(0.10f);
    text1->setPosition(pos);
    text1->setDrawMode(osgText::TextBase::TEXT);
    text1->setAxisAlignment(osgText::TextBase::SCREEN);
    text1->setText(Text);
    text1->setColor(Color);
    return text1;
}


//////////////////////////////////////////////////////////////////////////
void  Line::SetRadius( double dbRadius )
{
    avAssert( dbRadius > 0.0 );
    _dbRadius = dbRadius;
    _cRadiusUniformPtr->set( (float)_dbRadius );
}

//////////////////////////////////////////////////////////////////////////
void Line::SetColor( const osg::Vec3 vColor )
{
    _cTintingColorUniformPtr->set( vColor );
}

//////////////////////////////////////////////////////////////////////////
void Line::UpdateBoundingBox()
{
    // Reset only when recomputing bounding box from all control points
    if ( m_nStartChangeFrom == 0 )
    {
        _cBoundingBox.init();
    }

    uint32_t nNumOfCtrlPoints = _cControlPoints.size();
    const osg::Vec3 vOffset = osg::Vec3( _dbRadius, _dbRadius, _dbRadius );
    for ( uint32_t i = m_nStartChangeFrom; i < nNumOfCtrlPoints; i++ )
    {
        _cBoundingBox.expandBy( _cControlPoints[i] + vOffset );
        _cBoundingBox.expandBy( _cControlPoints[i] - vOffset );
    }

    // Update drawables
    for ( uint32_t i = 0; i < _geode->getNumDrawables(); i++ )
    {
        _geode->getDrawable(i)->dirtyBound();
    }
}

/////////////////////////
void Line::MakeSpline(std::vector<InterpolatedPoint>&  cInterpolatedPoints)
{

    if ( _cControlPoints.empty() )
    {
        return;
    }
}

/////////////////////////////////////////////////
void Line::Update()
{
    avAssert( _cTextureLength.size() == _cControlPoints.size() );
    avAssert( _dbSegmentLength > 0.0 );
    
    SaveOriginalEndPoints();

	TuneControlPoints ( );

    // Update bounding box
    UpdateBoundingBox();
    std::vector<InterpolatedPoint>  cInterpolatedPoints;
    // Create new segments
    MakeSpline(cInterpolatedPoints);

#if 0
    // invalidate cache
    m_nSearchSegmentStart = -1;
#endif
    // Optimize segment points
    OptimizeSegmentPoints(cInterpolatedPoints);

    // Compute bones
    CalcBonesTransforms();

    // Update drawables
    if ( _cControlPoints.empty() == false ) 
    {
        UpdateDrawablesCommon();
    }
    RestoreOriginalEndPoints();
    // Show line if it has enough number of points
    setNodeMask( !_bShowLine || _cControlPoints.size() < 2 ? 0 : DEFAULT_NODE_MASK );
}

//////////////////////////////////////////////////////////////////////////
void Line::CalcBonesTransforms()
{
    uint32_t		nStartIndex     = 0;
    osg::Vec3d			vUp				= CalcBaseRingOrientation(); 

    avAssert( _cBones.size() >= m_nStartSegmentPointsChangeFrom );
    _cBones.resize(_cSegmentPoints.size());
    if ( m_nStartSegmentPointsChangeFrom > 0 )
    {
        // avAssert( _eLineMode == LINE_MODE_DRUM );
        const CBone& cLastBone = _cBones [ m_nStartSegmentPointsChangeFrom - 1 ];
        vUp = cLastBone.vUp;
    } 

    osg::Matrix mBoneMatrix;
    uint32_t nNumOfBones = _cBones.size();
    bool bFirstBone = true;
    osg::Vec3d vPrevDirStraight;
    for ( uint32_t nIndex = m_nStartSegmentPointsChangeFrom; nIndex < nNumOfBones; nIndex++ )
    {
        CBone&						cBone	= _cBones[ nIndex ];
        const InterpolatedPoint&	cPoint	= _cSegmentPoints[ nIndex ];

        // Compute bone matrix
        // Now try to calc that matrix even if vPositions of neighbour points are the same
        // For the last point just use the previously calculated matrix with new center
        float fRadRatio = 1.0f;
        bool MatrixCalculated (false);
        for ( uint32_t NextPointIndex = nIndex + 1; NextPointIndex < nNumOfBones; NextPointIndex++ )
        {
            if ((cPoint.vPosition - _cSegmentPoints[ NextPointIndex ].vPosition).length2() > 0.0) // points are different
            {
                vUp = LookAt( mBoneMatrix, cPoint.vPosition, _cSegmentPoints[ NextPointIndex ].vPosition, vUp, bFirstBone, vPrevDirStraight, fRadRatio );
                bFirstBone = false;
                MatrixCalculated = true;
                break;
            }
        }
        if (!MatrixCalculated) // we get to the end so...
        {
            MakeLastBoneMatrix(mBoneMatrix, cPoint.vPosition, vUp, vPrevDirStraight);
        }

        // Rotate every second chain ring
        RotateEverySecondChainRing(mBoneMatrix, cPoint.bRotation );

        // Extract rotation quaternion and position from the matrix
        const double* pMatrixData = mBoneMatrix.ptr();
        cBone.vPosition[ 0 ] = pMatrixData[ 12 ];
        cBone.vPosition[ 1 ] = pMatrixData[ 13 ];
        cBone.vPosition[ 2 ] = pMatrixData[ 14 ];
        cBone.cRotation.set( mBoneMatrix );

        cBone.fTextureOffset = cPoint.fTextureOffset;
        cBone.m_fLinearLength = cPoint.m_fLinearLength;

        cBone.AdditionalPerElementData.set ( cPoint.m_fType, cPoint.m_fRadiusFactor / cg::bound(fRadRatio, 0.25f, 1.0f), 0.0f, 0.0f );

        // Set up vector
        cBone.vUp = vUp;

        // Set transformations for peewee sockets
        if ( nIndex == 0 )
        {
            SetEndpointMatrix ( 0, mBoneMatrix );
        } 
        else if ( nIndex == nNumOfBones - 1 ) 
        {
            SetEndpointMatrix ( 1, mBoneMatrix );
        }
    }
}

//////////////////////////////////////////////////////////////////////////
void Line::SetTexture(const char* szTextureFile)
{
    SetImage( avCore::GetDatabase()->LoadImage( szTextureFile ) );
}

//////////////////////////////////////////////////////////////////////////
void Line::SetImage( osg::Image* pImage )
{
    if ( pImage && pImage != _image.get() )
    {
        _image = pImage;

        osg::Texture2D* texture = new osg::Texture2D();
        texture->setWrap( osg::Texture2D::WRAP_S, osg::Texture2D::REPEAT );
        texture->setWrap( osg::Texture2D::WRAP_T, osg::Texture2D::REPEAT );
        texture->setImage(_image.get());

        osg::StateSet* stateset = _geode->getOrCreateStateSet();
        stateset->setNestRenderBins(false);
        stateset->setTextureAttribute(0, texture, osg::StateAttribute::ON );
    }
}

//////////////////////////////////////////////////////////////////////////
void Line::SetPosition( const osg::Vec3& cPosition )
{
#if 0
    // Save position and default rotation
    SetGeographicPositionAndRotation( cPosition, avCore::TaitBryanRotation() );
#endif
}

//////////////////////////////////////////////////////////////////////////
void Line::SetWorldTransformationMatrix( const osg::Matrix& cMatrix )
{
        setMatrix( osg::Matrixd::translate ( cMatrix.getTrans() ) );
}

//////////////////////////////////////////////////////////////////////////
void Line::ShowVisualModel( bool bShow /*= true*/ )
{
    _bShowLine = bShow;
    setNodeMask( _bShowLine && !_cControlPoints.empty() ? DEFAULT_NODE_MASK : 0 );
}


void Line::FillTextureLength( float StartTextureLength, bool IsIncreasingFromMasterLine )
{
    _cTextureLength.resize(_cControlPoints.size());
    if ( !_cTextureLength.empty() )
    {
        _cTextureLength.back() = StartTextureLength;
        for ( unsigned int i = _cTextureLength.size() - 1; i > 0 ; i--) //_cTextureLength.size() - 1 not negative
        {
            float SegmentLength = ( _cControlPoints [ i ] - _cControlPoints [ i - 1] ).length();
            StartTextureLength += IsIncreasingFromMasterLine ? SegmentLength : -SegmentLength;
            _cTextureLength [ i - 1 ] = StartTextureLength;
        }
    }
}

void Line::RemoveStrictlyCoincidentCP()
{
	// Remove strictly coincident control points. 
	// We believe that this is actually a very rare case so vector.erase is ok here.
	for ( unsigned int i = 1; i < _cControlPoints.size(); )
	{
		if ((_cControlPoints[i]-_cControlPoints[i-1]).length2()<=0.0001) // 1cm
		{
			_cControlPoints.erase(_cControlPoints.begin()+i);
			_cTextureLength.erase(_cTextureLength.begin()+i);
		}
		else
		{
			i++;
		}
	}
}


osg::Vec3d Line::CalcBaseRingOrientation()
{

    return osg::Vec3d();
}

void Line::FillTextureLengthForward()
{
    // There's no sense to call this method if _cTextureLength is empty now
    // we will need to rewrite it anyway
    avAssert(_cTextureLength.size() >= m_nStartChangeFrom && m_nStartChangeFrom > 0); 
    _cTextureLength.resize(_cControlPoints.size());
    for ( unsigned int i = m_nStartChangeFrom ; i < _cTextureLength.size(); i++) 
    {
        float SegmentLength = ( _cControlPoints [ i ] - _cControlPoints [ i - 1] ).length();
        _cTextureLength [ i ] = _cTextureLength [ i - 1 ] + ( -SegmentLength );
    }
}

void Line::SaveOriginalEndPoints()
{
    m_fTexLenStart = 0.0f; 
    m_fTexLenEnd = 0.0f;
    if (!_cTextureLength.empty())
    {
        m_fTexLenStart = _cTextureLength.front();
        m_fTexLenEnd = _cTextureLength.back();
    }
    if (!_cControlPoints.empty())
    {
        m_CPStart = _cControlPoints.front();
        m_CPEnd = _cControlPoints.back();
    }
}

void Line::RestoreOriginalEndPoints()
{
    if (!_cTextureLength.empty())
    {
        _cTextureLength.front() = m_fTexLenStart;
        _cTextureLength.back() = m_fTexLenEnd;
    }
    if (!_cControlPoints.empty())
    {
        _cControlPoints.front() = m_CPStart;
        _cControlPoints.back() = m_CPEnd;
    }
}

void Line::UpdateDrawablesCommon()
{
    if (_cBones.empty())
    {
        return;
    }

    int BonesMapWidthInElements = m_nBonesMapWidth / 3;
    uint32_t nNumOfBones = _cBones.size() + 1; // +1 add the last point one more time before the first one
    if (nNumOfBones>unsigned int(BonesMapWidthInElements*m_nBonesMapHeight))
    {
        m_nBonesMapHeight = ( nNumOfBones + BonesMapWidthInElements - 1 ) / BonesMapWidthInElements;
        if (m_nBonesMapHeight > 4096)
        {
            m_nBonesMapHeight = 4096;
            nNumOfBones = BonesMapWidthInElements * m_nBonesMapHeight - 1;
        }
        ResizeBonesImage( m_BonesMapTexture->getImage() );
    }

    osg::Image & image = *m_BonesMapTexture->getImage();
    float *dst = (float *)image.data();
    // add the last point one more time before the first one
    {
        const CBone& cBone = _cBones.back();
        const osg::Vec4d& Quat = cBone.cRotation.asVec4(); 
        *dst++ = Quat.x();
        *dst++ = Quat.y();
        *dst++ = Quat.z();
        *dst++ = Quat.w();

        const osg::Vec3f& Pos = cBone.vPosition; 
        *dst++ = Pos.x();
        *dst++ = Pos.y();
        *dst++ = Pos.z();
        *dst++ = cBone.fTextureOffset;

        const osg::Vec4f& AdditionalPerElementData = cBone.AdditionalPerElementData;
        *dst++ = AdditionalPerElementData.x(); // Type of texture
        *dst++ = AdditionalPerElementData.y(); // Factor of radius
        *dst++ = AdditionalPerElementData.z();
        *dst++ = AdditionalPerElementData.w();
    }
    uint32_t PointsClamped ( 0 );
    for ( auto It = _cBones.begin(); It != _cBones.end(); ++It )
    {
        const CBone& cBone = *It;
#if 0
        if ( m_bIsLengthClampEnabled && cBone.m_fLinearLength < m_fClampBefore )
        {
            PointsClamped++;
            continue;
        }
#endif

        const osg::Vec4d& Quat = cBone.cRotation.asVec4(); 
        *dst++ = Quat.x();
        *dst++ = Quat.y();
        *dst++ = Quat.z();
        *dst++ = Quat.w();

        const osg::Vec3f& Pos = cBone.vPosition; 
        *dst++ = Pos.x();
        *dst++ = Pos.y();
        *dst++ = Pos.z();
        *dst++ = cBone.fTextureOffset;

        const osg::Vec4f& AdditionalPerElementData = cBone.AdditionalPerElementData;
        *dst++ = AdditionalPerElementData.x(); // Type of texture
        *dst++ = AdditionalPerElementData.y(); // Factor of radius
        *dst++ = AdditionalPerElementData.z();
        *dst++ = AdditionalPerElementData.w();
    }
    image.dirty();

    osg::Geometry* geometry = _geode->getDrawable(0)->asGeometry();
    if (geometry)
    {
        uint32_t LinksNumToDraw = 0;
        if (_cBones.size()>0 && _cBones.size()>PointsClamped+1)
        {
            // -1 for do not start one more link from final point
            LinksNumToDraw = _cBones.size() - PointsClamped - 1;
        }
        geometry->getPrimitiveSet(0)->setNumInstances(LinksNumToDraw); 
        // setNumInstances(0) draws one instance in OSG anyway so we need to stop that
        _geode->setNodeMask(LinksNumToDraw==0?0:0xFFFFFFFF);
    }
    UpdateDrawables();
    // Update relevant geometrical features like hairs or foils
    UpdateHairsAndFoils();
}

void Line::RemoveOldPoints( std::vector<InterpolatedPoint>& Points, const std::vector<InterpolatedPoint>& cInterpolatedPoints )
{
    if (m_nStartChangeFrom > 0)
    {
        // Partial rebuild
        avAssert(!cInterpolatedPoints.empty());
        float ClearUpTo = cInterpolatedPoints.front().m_fLinearLength;

        while ( !Points.empty() && Points.back().m_fLinearLength <= ClearUpTo )
        {
            Points.pop_back();
        }
    }
    else
    {   
        // Complete rebuild
        // Remove all segment points
        Points.clear();
    }
}

osg::TextureRectangle* Line::createBonesMap()
{
        osg::Image* image = new osg::Image;
        image->setDataVariance(osg::Image::DYNAMIC); 
        ResizeBonesImage( image );

        osg::TextureRectangle* texture = new osg::TextureRectangle;

        //texture->setTextureSize(m_nBonesMapWidth, m_nBonesMapHeight);
        texture->setImage(image);
        texture->setFilter(osg::Texture::MIN_FILTER, osg::Texture::NEAREST);
        texture->setFilter(osg::Texture::MAG_FILTER, osg::Texture::NEAREST);
        texture->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
        texture->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
        texture->setUseHardwareMipMapGeneration(false);

        return texture;
}

void Line::ResizeBonesImage( osg::Image* image )
{
    const size_t data_size = m_nBonesMapWidth * m_nBonesMapHeight * 4;
    float* data = new float[data_size];
    memset(data, 0, sizeof(float) * data_size);
    image->setImage(m_nBonesMapWidth, m_nBonesMapHeight, 0,
        GL_RGBA32F_ARB, // internalFormat
        GL_RGBA,        // pixelFormat,
        GL_FLOAT,       // dataType,
        (unsigned char*)data,
        osg::Image::USE_NEW_DELETE);
}


// get position of point on line
bool Line::GetLinePointPos( float fLineLength, osg::Vec3 & vOutPos, float & vRadCur, osg::Vec3 * pvUp )
{
    // not initialized
    if (m_nSearchSegmentStart < 0 && _cSegmentPoints.size() > 1)
    {
        m_iRaiseDir = osg::sign(_cSegmentPoints[1].m_fLinearLength - _cSegmentPoints[0].m_fLinearLength);
        m_nSearchSegmentStart = 0;
    }

    // benefit from cache
    unsigned start = 0;
    int dir = +1;
    if (m_nSearchSegmentStart >= 0)
    {
        start = m_nSearchSegmentStart;
        dir = m_iRaiseDir;
    }

    // search
    int i0 = start;
    do
    {
        int i1 = i0 + 1;

        const float fMin = osg::minimum(_cSegmentPoints[i0].m_fLinearLength, _cSegmentPoints[i1].m_fLinearLength);
        const float fMax = osg::maximum(_cSegmentPoints[i0].m_fLinearLength, _cSegmentPoints[i1].m_fLinearLength);
        if (fLineLength >= fMin && fLineLength <= fMax)
        {
            const float fRatio = (fLineLength - _cSegmentPoints[i0].m_fLinearLength) / (_cSegmentPoints[i1].m_fLinearLength - _cSegmentPoints[i0].m_fLinearLength);
            vOutPos = cg::lerp01( _cSegmentPoints[i0].vPosition, _cSegmentPoints[i1].vPosition, fRatio);
            vRadCur = cg::lerp01( _cSegmentPoints[i0].m_fRadiusFactor, _cSegmentPoints[i1].m_fRadiusFactor, fRatio);
            if (pvUp)
            {
                *pvUp = cg::lerp01( _cBones[i0].vUp, _cBones[i1].vUp, fRatio);
                // normalization is commented because it will be done later on GPU-side
                //pvUp->normalize();
            }
            m_nSearchSegmentStart = i0;
            return true;
        }

        // step next
        i0 = i0 + dir;
        if (i0 == _cSegmentPoints.size() - 1)
            i0 = 0;
        else if (i0 == -1)
            i0 = _cSegmentPoints.size() - 2;
    } while (i0 != start);

    // nothing's found
    return false;
}
