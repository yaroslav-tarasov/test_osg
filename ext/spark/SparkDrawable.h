#ifndef H_SPARKDRAWABLE
#define H_SPARKDRAWABLE

/** The spark drawable contains one or more particle systems of the same base type */
class SparkDrawable : public osg::Drawable
{
public:
    class DeferredSystemHandler;
    struct SortParticlesOperator;
    
    struct ImageAttribute
    {
        osg::ref_ptr<osg::Image> image;
        GLuint type;
        GLuint clamp;
    };

    typedef std::map<std::string, GLuint>         TextureIDMap;
    typedef SPK::SPK_ID (*CreateBaseSystemFunc)( const TextureIDMap&, int width, int height );

public:
    SparkDrawable();
    SparkDrawable( const SparkDrawable& copy,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY );
    META_Object( osg, SparkDrawable )
    
    bool               isValid() const; 
    unsigned int       getNumParticles() const;
    
    void               setBaseSystemCreator( CreateBaseSystemFunc func, bool useProtoSystem=false );

    
    void               setBaseSystemID( SPK::SPK_ID id );
    SPK::SPK_ID        getBaseSystemID() const ;
    
    void               setSortParticles( bool b ) ;
    bool               getSortParticles() const ;
    
    void               setAutoUpdateBound( bool b ) ;
    bool               getAutoUpdateBound() const ;
    
    SPK::System*       getProtoSystem() ;
    const SPK::System* getProtoSystem() const ;
    
    SPK::System*       getParticleSystem( unsigned int i ) ;
    const SPK::System* getParticleSystem( unsigned int i ) const ;
    unsigned int       getNumParticleSystems() const ;
    
    void               addExternalParticleSystem( SPK::System* system ) ;
    void               destroyParticleSystem( SPK::System* system, bool removeFromList=true );
    
    void               setGlobalTransformMatrix( const osg::Matrix& matrix, bool useOffset=false );
    void               setTransformMatrix( SPK::System* system, const SPK::Vector3D& pos, const SPK::Vector3D& rot,
                             float angle, bool useOffset=false );
    
    /** Add a new system cloned from the base system and return the index for deferred use */
    unsigned int       addParticleSystem( const osg::Vec3& p=osg::Vec3(), const osg::Quat& r=osg::Quat() );
    
    /** Add an image for the creator func to use, must be done before the creator func starting */
    void               addImage( const std::string& name, osg::Image* image, GLuint type=GL_RGB, GLuint clamp=GL_CLAMP );
    
    /** Update the system, will be called by the SparkUpdatingHandler */
    virtual bool       update( double currentTime, const osg::Vec3d& eye );
    
#if OSG_MIN_VERSION_REQUIRED(3,3,2)
	virtual osg::BoundingBox computeBoundingBox() const;
#else
	virtual osg::BoundingBox computeBound() const;
#endif
    virtual void       drawImplementation( osg::RenderInfo& renderInfo ) const;
    
protected:
    virtual ~SparkDrawable();
    
    struct Private;
    Private*     p_; 
    
    GLuint             compileInternalTexture( osg::Image* image, GLuint type, GLuint clamp ) const;
    void               convertData( osg::Image* image, GLuint type, unsigned int numCurrent, unsigned int numRequired ) const;

    mutable unsigned int    _activeContextID;
    double                  _lastTime;
    bool                    _sortParticles;
    bool                    _useProtoSystem;
    bool                    _autoUpdateBound;
    mutable bool            _dirty;
};

#endif
