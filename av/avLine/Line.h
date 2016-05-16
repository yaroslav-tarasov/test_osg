#pragma once

#include "av/avCore/avCore.h"


namespace avLine
{

    
class  Line : public osg::MatrixTransform
{
protected:

    // Interpolated point
    class InterpolatedPoint
    {
    public:
        InterpolatedPoint() : fTextureOffset( 0.0f ), m_fLinearLength(0.0f), m_bIsControlPoint( false ), bRotation( false ), m_bDeepUnderWater(false), m_fType (0.0f), m_fRadiusFactor (1.0f) {}
        InterpolatedPoint( osg::Vec3 v, float o, bool b, bool bRot ) : vPosition( v ), fTextureOffset( o ), m_fLinearLength( o ), m_bIsControlPoint( b ), bRotation( bRot ), 
                                                                m_bDeepUnderWater(false), m_fType (0.0f), m_fRadiusFactor (1.0f) {}
        osg::Vec3						vPosition;
        float							fTextureOffset;
        bool							m_bIsControlPoint;
        bool							bRotation;		// Used by chain to rotate every second ring
        bool                            m_bDeepUnderWater;
        float                           m_fType;
        float                           m_fRadiusFactor;
        float                           m_fLinearLength;
    };

    // Bone
    class CBone
    {
    public:
            CBone() : fTextureOffset( 0.0 ), vUp( 0, 1, 0 ) {}

        osg::Quat						cRotation;
        osg::Vec3						vPosition;
        double							fTextureOffset;
        osg::Vec3d						vUp;
        osg::Vec4f                      AdditionalPerElementData;
        float                           m_fLinearLength;
    };

    // Bounding box callback
    class CBoundingBoxCallback : public osg::Drawable::ComputeBoundingBoxCallback
    {
    public:
        CBoundingBoxCallback( const osg::BoundingBox& BB ) : m_BB( BB ) {}
    private:
        virtual osg::BoundingBox computeBound( const osg::Drawable& cDrawable ) const override { return m_BB; }
        const osg::BoundingBox& m_BB;
    };


public:

    void								Update();
    inline void							SetSegmentLength(double dbLength) { _dbSegmentLength = dbLength; }
    inline void                         SetSegmentExtraOffset(double dbLength) { _dbSegmentExtraOffset = dbLength; }
    const std::vector<osg::Vec3d>&		GetControlPoints() { return _cControlPoints; }
    virtual void						SetRadius( double dbRadius );
    void								SetColor( const osg::Vec3 vColor );
    void								SetTexture(const char* szTextureFile);
    void								SetImage( osg::Image* pImage );
    inline osg::Image*					GetImage() { return _image.get(); }    
    inline double						GetRadius() const { return _dbRadius; }
    
    void								SetPosition( const osg::Vec3& cPosition );
    virtual void						SetWorldTransformationMatrix( const osg::Matrix& cMatrix );
    void								ShowVisualModel( bool bShow = true );
    inline const osg::BoundingBox&		GetBoundingBox() const { return _cBoundingBox; }
    inline bool                         IsEmpty() const { return _cControlPoints.empty(); }
    void                                FillTextureLength ( float StartTextureLength, bool IsIncreasingFromMasterLine );
    void                                FillTextureLengthForward ( );
    virtual void                        UpdateTextureLengthOffset (Line* MasterLine) {}

protected:  
                                        Line( uint32_t nID );
    virtual								~Line();

    inline CBoundingBoxCallback*		GetBoundingBoxCallback() { return _cBoundingBoxCallbackPtr.get(); }
    void						        UpdateDrawablesCommon();
    virtual void						UpdateDrawables() = 0;
    void                                RemoveOldPoints ( std::vector<InterpolatedPoint>& Points, const std::vector<InterpolatedPoint>& cInterpolatedPoints );

    bool                                GetLinePointPos( float fLineLength, osg::Vec3 & vOutPos, float & vRadCur, osg::Vec3 * pvUp = NULL );


private:
    void								UpdateBoundingBox();
    void								CalcBonesTransforms();
    void						        MakeSpline(std::vector<InterpolatedPoint>&  cInterpolatedPoints);

    virtual void                        TuneControlPoints() = 0;
    virtual void						OptimizeSegmentPoints( std::vector<InterpolatedPoint>& cInterpolatedPoints ) = 0;
    virtual void						RotateEverySecondChainRing ( osg::Matrix& BoneMatrix, bool Rotation ) {}
    virtual void                        SetEndpointMatrix ( uint32_t nIndex, const osg::Matrix& EndpointMatrix ) {}

    virtual void                        UpdateHairsAndFoils() {}

	void								RemoveStrictlyCoincidentCP();
    osg::Vec3d			                CalcBaseRingOrientation ();
    void                                SaveOriginalEndPoints ();
    void                                RestoreOriginalEndPoints ();
    osg::TextureRectangle*              createBonesMap ();
    void                                ResizeBonesImage (osg::Image* image); 

protected:
    bool                                _bShowLine;
    double								_dbRadius;
    std::vector<osg::Vec3d>				_cControlPoints;
    std::vector<float>					_cTextureLength;
    std::vector<InterpolatedPoint>		_cSegmentPoints;
    std::vector<InterpolatedPoint>		_cOldSegmentPoints;
    std::vector<CBone>					_cBones;
    double								_dbSegmentLength;
    double                              _dbSegmentExtraOffset;
    osg::ref_ptr<osg::Geode>			_geode;
    osg::ref_ptr<osg::Image>			_image;
    osg::ref_ptr<osg::Group>			_cControlPointsVisual;
    osg::BoundingBox					_cBoundingBox;
    osg::ref_ptr<CBoundingBoxCallback>	_cBoundingBoxCallbackPtr;
    osg::ref_ptr<osg::Uniform>			_cRadiusUniformPtr;
    osg::ref_ptr<osg::Uniform>			_cTintingColorUniformPtr;

    int                                 m_nBonesMapWidth, m_nBonesMapHeight;
    osg::ref_ptr<osg::TextureRectangle> m_BonesMapTexture; 

    bool                                m_bUseSplines;
    bool                                m_bNeedToUpdateDrumLine;
    float                               m_fTexLenStart, m_fTexLenEnd;
    osg::Vec3d                          m_CPStart, m_CPEnd;
    uint32_t                            m_nStartChangeFrom;
    uint32_t                            m_nStartSegmentPointsChangeFrom;


    int                                 m_nSearchSegmentStart;
    int                                 m_iRaiseDir;
};

};

namespace osgLine = avLine;