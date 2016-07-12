#include "stdafx.h"

//#include <avDebug.h>
#include "av/avCore/Database.h"

#include "Animation.h"
#include "AnimationPrototype.h"



namespace avAnimation 
{

bool readXFile ( std::istream * is, AnimationLoader* Loader );

bool IsEqual ( float delta, const osg::Matrixf& A, const osg::Matrixf& B )
{
    const float *pA = A.ptr ();
    const float *pB = B.ptr ();
    for ( int i = 0; i < 16; i++ )
    {
        if ( fabs ( *pA++ - *pB++ ) > delta )
        {
            return false;
        }
    }
    return true;
}

class NamedDrawableCullCallback : public osg::Drawable::CullCallback
{
    bool cull(osg::NodeVisitor* nv, osg::Drawable* drawable, osg::RenderInfo* renderInfo) const 
    { 
        const osg::NodePath& np = nv->getNodePath();
        for ( osg::NodePath::const_reverse_iterator It = np.rbegin(); It != np.rend(); It++ )
        { 
            const AnimationObject* AO = dynamic_cast <const AnimationObject*> ( *It );
            if ( AO )
            {
                return AO -> IsNameCulled( drawable->getName().c_str() );
            }
        }
        return false; 
    }
};

class AnimatedDrawableComputeBoundingBoxCallback : public osg::Drawable::ComputeBoundingBoxCallback
{
    osg::BoundingBox computeBound(const osg::Drawable &drawable) const  
    { 
        osg::BoundingBox BB;
#if OSG_MIN_VERSION_REQUIRED(3,3,2)
		BB = drawable.computeBoundingBox();
#else
		BB = drawable.computeBound();
#endif
        
        // Let's estimate possible BB change due to animation matrices application
        // supposed that all the animated model drawables are created relative to the model zero
        // and could be arbitrary rotated around it.
        osg::Vec3f diff = -BB.center();

        diff.x() = diff.x() > 0 ? diff.x() : -diff.x();
        diff.y() = diff.y() > 0 ? diff.y() : -diff.y();
        diff.z() = diff.z() > 0 ? diff.z() : -diff.z();

        diff += ( BB._max - BB._min ) * 0.5f; // substitute half size so diff now is vector to the most distant vertex

        float MaxRadius = diff.length ();
        // At the moment take the upper hemisphere only (assume the rest is under the deck)
        BB.set ( -MaxRadius, -MaxRadius, 0.0f, MaxRadius, MaxRadius, MaxRadius );

        return BB; 
    }
};

std::map<std::string, AnimationPrototype*> AnimationPrototype::m_Cache;

AnimationPrototype::AnimationPrototype ( ResourceLoaderPtr ResourceLoader ) :
 m_bTextureDefined ( false ),
 m_nMatrixIndex ( 0 ),
 m_bNeedToOptimize (true),
 m_bThisMatrixIsNotUsed (false),
 m_bAllEqualToInitial (true),
 m_pLastBoneAdded (NULL),
 m_ResourceLoader ( ResourceLoader )
{
    m_CurAnimationSetForLoad = m_Animations.end(); 
    // Make a root identity bone as place for the collection of unattached bones
    m_RootBone = CreateBone( "Unattached root" );
    m_BonesLoadingStack.push_back(m_RootBone.get());

    osg::StateSet* stateset = getOrCreateStateSet();
    stateset->setMode(GL_VERTEX_PROGRAM_POINT_SIZE,osg::StateAttribute::ON);
    stateset->setMode(GL_CULL_FACE,osg::StateAttribute::ON);

    osg::Program* program = new osg::Program;
    stateset->setAttribute(program);

    // get shaders from source
    program->addShader( m_ResourceLoader->getShader ( "Animation.vs", osg::Shader::VERTEX   ));
    program->addShader( m_ResourceLoader->getShader ( "Animation.fs", osg::Shader::FRAGMENT ));
}

AnimationPrototype::~AnimationPrototype ( ) 
{
    m_Cache.erase ( m_Name );
}

void AnimationPrototype::Update ( float TimeFromAnimationStart, const AnimationElement *MainAE, osg::Uniform *Table, ExternalBonesMap & extMatrixMap,
                                                                        const AnimationElement *NextAE, float ToNextAnimationBlendFactor )
{
    //_uniformBoneMatrices->setElement( unsigned int index, const osg::Matrixf& m4 );
    if ( !m_RootBone.valid ( ) )
        return;

    ResetBones ( m_RootBone.get() );
    // Precalculate some timings for main and additional animations

    UpdateBones ( TimeFromAnimationStart, *MainAE, 1.0f );
    if ( NextAE ) // Blending to the next AnimationElement is active
        UpdateBones ( 0.0f, *NextAE, ToNextAnimationBlendFactor );

    // Recalculate Matrices
    osg::Matrixf M;
    RecalcBones ( m_RootBone.get(), M, Table, extMatrixMap );
}

void AnimationPrototype::OptimizeBonesTree ( )
{

    // Remove unused (existing only for useful control) matrices here
    // 1. Remove root bone if there were no bones added manually
    if ( m_RootBone -> GetChildren().size ( ) == 1 )
    { 
        //avLog(MT_INFO, "Remove bone: %s\n", m_RootBone -> GetName().c_str() );
        m_BoneMap.erase ( m_RootBone -> GetName() );
        m_RootBone = m_RootBone -> GetChildren() [ 0 ]; // everything removed automatically
    }
    // 2. Remove the bone if there is no vertices dependent on it (_ResultIndex==-1) or on deeper bones 
    RemoveRedundantBones ( m_RootBone.get(), 0 );

    // The following lines are for debugging only
    osg::Matrixf M;
    RecalcMTBM ( m_RootBone.get(), M );

}

const AnimationSet* AnimationPrototype::GetAnimationSet ( const std::string &AnimationName ) const
{
    AnimationCollection::const_iterator ACI = m_Animations.find ( AnimationName );
    if ( ACI == m_Animations.end ( ) )
        return 0;
    return &ACI -> second;
}


void AnimationPrototype::ResetBones ( Bone *bone )
{
    bone -> Reset(); 

    for ( unsigned int i = 0; i < bone -> GetChildren().size(); i++ )
        ResetBones ( bone -> GetChildren()[ i ].get() );
}

void AnimationPrototype::UpdateBones ( float TimeFromAnimationStart, const AnimationElement &AE, float CurNextInfluenceBlendFactor )
{
    // Main animation
    float CurPosInFrames = fmod ( TimeFromAnimationStart, AE.m_fDuration ) / AE.m_fDuration * AE.m_pAnimationSet->m_LengthInFrames;
    for ( std::vector<AnimationKey>::const_iterator It = AE.m_pAnimationSet->begin(); It != AE.m_pAnimationSet->end(); ++It )
        It -> m_pBone -> Animate ( CurPosInFrames, CurNextInfluenceBlendFactor, *It );
    
    // Additional animations
#if 1
    for ( std::vector<AnimationSubElement>::const_iterator ASE_It = AE.m_SubElements.begin(); ASE_It != AE.m_SubElements.end(); ++ASE_It )
    {   
        // Precalculate some timings
        float TimeFromAdditionalAnimationStart = fmod ( TimeFromAnimationStart, ASE_It->m_fPauseLength + ASE_It->m_fDuration ) - ASE_It->m_fPauseLength;
        if ( TimeFromAdditionalAnimationStart < 0.0f )
            continue;
        float MainAdditionalInfluenceBlendFactor = 1.0f;
        if ( TimeFromAdditionalAnimationStart < ASE_It->m_fBlendBorderLength )
            MainAdditionalInfluenceBlendFactor = TimeFromAdditionalAnimationStart / ASE_It->m_fBlendBorderLength;
        else if ( ASE_It->m_fDuration - TimeFromAdditionalAnimationStart < ASE_It->m_fBlendBorderLength )
            MainAdditionalInfluenceBlendFactor = ( ASE_It->m_fDuration - TimeFromAdditionalAnimationStart ) / ASE_It->m_fBlendBorderLength;

        float CurPosInFrames = fmod ( TimeFromAdditionalAnimationStart, ASE_It->m_fDuration ) / ASE_It->m_fDuration * 
                                                                                        ASE_It ->m_pAnimationSet->m_LengthInFrames;
        for ( std::vector<AnimationKey>::const_iterator It = ASE_It -> m_pAnimationSet->begin(); It != ASE_It -> m_pAnimationSet->end(); ++It )
            It -> m_pBone -> Animate ( CurPosInFrames, MainAdditionalInfluenceBlendFactor * CurNextInfluenceBlendFactor, *It );
    }
    
#endif
}


void AnimationPrototype::RecalcBones ( Bone *bone, const osg::Matrixf &Accumulated, osg::Uniform *Table, ExternalBonesMap & m_extMatrixMap )
{
    // update amtrix
    osg::Matrixf ThisNodeAccumulated(bone->GetCurrentMatrix());
    ThisNodeAccumulated.postMult(Accumulated);

    // update uniform if needed
    if (bone->GetMatrixIndex() >= 0)
        Table->setElement(bone->GetMatrixIndex(), bone->GetModelToBoneMatrix() * ThisNodeAccumulated);

    // update external trackable matrices
    ExternalBonesMap::iterator itExtBone = m_extMatrixMap.find(bone->GetName());
    if (itExtBone != m_extMatrixMap.end())
        itExtBone->second->setMatrix(ThisNodeAccumulated);

    // go down
    for (unsigned i = 0; i < bone->GetChildren().size(); ++i)
        RecalcBones(bone->GetChildren()[i].get(), ThisNodeAccumulated, Table, m_extMatrixMap);
}

bool AnimationPrototype::RemoveRedundantBones ( Bone *bone, Bone *FatherBone )
{
    // Have to begin from the children
    for ( unsigned int i = 0; i < bone -> GetChildren().size();  )
    {
        if ( !RemoveRedundantBones ( bone -> GetChildren()[ i ].get(), bone ) )
        {
            i++;
        }
    }

    if ( FatherBone && bone -> GetMatrixIndex() == -1 && bone -> GetChildren().empty ( ) )
    {
        //avLog(MT_INFO, "Remove bone: %s\n", bone -> GetName().c_str() );
        m_BoneMap.erase ( bone -> GetName() );
        for ( std::vector<osg::ref_ptr<Bone>>::const_iterator It = FatherBone -> GetChildren().begin (); It != FatherBone -> GetChildren().end(); ++It )
        {
            if ( (*It) -> GetName() == bone -> GetName() )
            {
                FatherBone -> GetChildren().erase ( It );
                break;
            }
        }
        return true;
    }
    return false;
}


void AnimationPrototype::RecalcMTBM ( Bone *bone, const osg::Matrixf &Accumulated )
{
    osg::Matrixf ThisNodeAccumulated ( bone -> GetInitialMatrix() * Accumulated ); 
    if ( bone -> GetMatrixIndex() >= 0 )
    {   
        if ( !IsEqual ( 0.01, osg::Matrixf::inverse ( ThisNodeAccumulated ), bone -> GetModelToBoneMatrix() ) )
        {
            //avLog(MT_INFO, "Recalculation of ModelToBoneMatrix gives different results.\n" );
        }
    }
    for ( unsigned int i = 0; i < bone -> GetChildren().size(); i++ )
    {
        RecalcMTBM ( bone -> GetChildren()[ i ].get(), ThisNodeAccumulated );
    }
}

AnimationPrototype *AnimationPrototype::CreateAnimationPrototype ( const char *Model, ResourceLoaderPtr ResourceLoader )
{
    std::map<std::string, AnimationPrototype *>::const_iterator It = m_Cache.find ( Model );
    if ( It == m_Cache.end () )
    {
        // Store even zero results for not to try to load them again
        
        AnimationPrototype *Result = new AnimationPrototype ( ResourceLoader );
        Result -> m_Name = Model;
        // Here we could choose the loader according to the Model extension. Now only readXFile from XFileParser is available
        if ( !avAnimation::readXFile ( ResourceLoader->getStream ( Model )->get (), Result ))
        {
            delete Result;
            Result = NULL;
        }
        m_Cache [ Model ] = Result;
        return Result;
    }
    return It -> second;
}

Bone* AnimationPrototype::CreateBone( const std::string& Name )
{
    Bone *bone = new Bone ( Name );
    m_BoneMap [ Name ] = bone;
    m_pLastBoneAdded = bone;
    return bone;
}

Bone* AnimationPrototype::GetBone( const std::string& Name )
{
    PrototypeBoneMap::iterator boneIt = m_BoneMap.find(Name);
    if (boneIt != m_BoneMap.end())
        return boneIt->second;
    return NULL;
}


void AnimationPrototype::AddBone ( const std::string& Name )
{
    m_BonesLoadingStack.back() -> GetChildren().push_back( CreateBone( Name ) );
}

void AnimationPrototype::SetBoneInitialMatrix ( const osg::Matrixf& M )
{
    if ( m_pLastBoneAdded )
    {
        m_pLastBoneAdded -> SetInitialMatrix( M );
    }
}

void AnimationPrototype::MoveLevelDown ( )
{
    m_BonesLoadingStack.push_back(m_pLastBoneAdded);
}

void AnimationPrototype::MoveLevelUp ( )
{
    m_pLastBoneAdded = m_BonesLoadingStack.back();
    m_BonesLoadingStack.pop_back();
}

void AnimationPrototype::SetIndexAndModelToBoneMatrix ( const std::string& Name, const osg::Matrixf& Mat )
{
    // Look for such a bone in the bone tree
    std::map<std::string, Bone *>::iterator It = m_BoneMap.find ( Name );
    Bone *bone = NULL;
    if ( It == m_BoneMap.end ( ) )
    { 
        // No such a bone in the bone tree. Try to add it to the root bone
        bone = CreateBone( Name );
        bone -> SetInitialMatrix ( osg::Matrixf::inverse ( Mat ) );
        m_RootBone -> GetChildren().push_back( bone );
    }
    else
    {
        bone = It->second;
    }
    bone->SetIndexAndModelToBoneMatrix ( m_nMatrixIndex, Mat );
    if ( m_nMatrixIndex > MaxBlendingMatricesPerModel )
    {
        //avLog( MT_INFO, "Number of blending matrices per model exceeds the maximum.\n" );
    }
}

void AnimationPrototype::AddDrawable( const std::string& Name, std::vector<osg::Vec3f>& Vertices, std::vector<osg::Vec3f>& Normals, std::vector<osg::Vec2f>& TxtCoord, std::vector<int>& Indices, std::map<std::string, std::vector<std::pair<int,float>>>& BonesMapOfVerticesWeghtsTables, const std::string& TextureName )
{
    int MaxWeightUsed = 0;

    // Optimization
#if 1
    // Remove duplicated vertices here
    // Just calculate unique vertices number
    std::vector<int> IndicesRemap;
    int UniqueVerticesNum ( 0 );
    {
        struct VertexCompare 
        {	
            VertexCompare ( const std::vector<osg::Vec3f> &Vertices, const std::vector<osg::Vec3f> &Normals, const std::vector<osg::Vec2f> &TxtCoord ) :
             _Vertices ( Vertices ),
             _Normals ( Normals ),
             _TxtCoord ( TxtCoord )
            {
            }
            bool operator()(const int &_Left, const int &_Right) const
            {	
                if ( _Vertices [ _Left ] < _Vertices [ _Right ] ) 
                    return true;  
                if ( _Vertices [ _Right ] < _Vertices [ _Left ] ) 
                    return false;  
                if ( _TxtCoord [ _Left ] < _TxtCoord [ _Right ] ) 
                    return true;  
                if ( _TxtCoord [ _Right ] < _TxtCoord [ _Left ] ) 
                    return false;  
                if ( _Normals [ _Left ] < _Normals [ _Right ] ) 
                    return true;  
                return false;  
            }
            const std::vector<osg::Vec3f> &_Vertices;
            const std::vector<osg::Vec3f> &_Normals;
            const std::vector<osg::Vec2f> &_TxtCoord; 
        };
        std::map<int,int,VertexCompare> MM ( VertexCompare ( Vertices, Normals, TxtCoord ) );
        // check the i-th vertex to be unique if yes then assign it next unique index and store it in MM
        // if not then takes the unique index that is already in MM and in any case fill the IndicesRemap
        for ( unsigned int i = 0; i < Vertices.size(); i++ )
        {
            IndicesRemap.push_back ( MM.insert ( std::pair <int,int>( i, (unsigned)MM.size() ) ).first -> second );
        }
        UniqueVerticesNum = MM.size();
    }

    // use IndicesRemap for proper remapping of all the data
    for ( unsigned int i = 0; i < Indices.size(); i++ )
    {
        Indices [ i ] = IndicesRemap [ Indices [ i ] ];
    }
    int AlreadyFilledIndex = -1;
    for ( unsigned int i = 0; i < Vertices.size() && AlreadyFilledIndex < UniqueVerticesNum; i++ )
    {
        int Index = IndicesRemap [ i ];
        if ( Index > AlreadyFilledIndex ) // new index
        {
            Vertices [ Index ] = Vertices [ i ];
            Normals [ Index ] = Normals [ i ];
            TxtCoord [ Index ] = TxtCoord [ i ];
            AlreadyFilledIndex = Index;
        }
    }
    _ASSERT ( AlreadyFilledIndex + 1 == UniqueVerticesNum );
    Vertices.resize ( UniqueVerticesNum );
    Normals.resize ( UniqueVerticesNum );
    TxtCoord.resize ( UniqueVerticesNum );
    std::vector<osg::Vec4ub> Matrix_Weights ( UniqueVerticesNum ); // filled by osg::Vec4ub ( )
    std::vector<osg::Vec4ub> Matrix_Indices ( UniqueVerticesNum );

    // Recalculate per bone weights
    for ( std::map<std::string, std::vector<std::pair<int,float>>>::iterator It = BonesMapOfVerticesWeghtsTables.begin ();
                                                                                    It != BonesMapOfVerticesWeghtsTables.end (); ++It )
    {
        std::vector<bool> HasBeenWritten ( UniqueVerticesNum ); //filled by false
        std::vector<std::pair<int,float>> NewTable;
        const std::vector<std::pair<int,float>> &OldTable = It -> second;
        for ( std::vector<std::pair<int,float>>::const_iterator ItOT = OldTable.begin(); ItOT != OldTable.end(); ++ItOT )
        {
            int NewIndex = IndicesRemap [ ItOT -> first ];
            if ( !HasBeenWritten [ NewIndex ] )
            {
                HasBeenWritten [ NewIndex ] = true;
                NewTable.push_back ( std::pair<int,float> ( NewIndex, ItOT -> second ) );
            }
        }
        It -> second = NewTable;        
    }

#endif
    // Recalculate from vertex weights per matrix to matrix weight per vertex
    for ( std::map<std::string, Bone *>::const_iterator BonesIt = m_BoneMap.begin(); BonesIt != m_BoneMap.end(); ++BonesIt )
    {
        if ( BonesIt -> second -> GetMatrixIndex() == -1 )
        {
            continue;
        }
        const std::vector<std::pair<int,float>> &Table = BonesMapOfVerticesWeghtsTables [ BonesIt -> second -> GetName() ];
        for ( std::vector<std::pair<int,float>>::const_iterator VertIt = Table.begin(); VertIt != Table.end(); ++VertIt )
        {
            unsigned char VertexWeight = VertIt -> second * 255.0f;
            osg::Vec4ub &Dest_Weights = Matrix_Weights [ VertIt->first ];
            osg::Vec4ub &Dest_Indices = Matrix_Indices [ VertIt->first ];

            int MinWeightIndex = 0;
            int i = 0;
            for ( ; i < 4; i++ )
            {
                if ( Dest_Weights._v [ i ] == 0 )
                { 
                    Dest_Weights._v [ i ] = VertexWeight;
                    Dest_Indices._v [ i ] = BonesIt -> second -> GetMatrixIndex();
                    break;
                }
                else if ( Dest_Weights._v [ i ] < Dest_Weights._v [ MinWeightIndex ] )
                {
                    MinWeightIndex = i;
                }
            }
            if ( i == 4 ) // We have more than 4 matrices for this vertex so replace the one with smallest weight
            { 
                //avLog( MT_INFO, "Vertex needs more than 4 matrices for blending. We do not support that now.\n" );
                if ( Dest_Weights._v [ MinWeightIndex ] < VertexWeight )
                {
                    Dest_Weights._v [ MinWeightIndex ] = VertexWeight;
                    Dest_Indices._v [ MinWeightIndex ] = BonesIt -> second -> GetMatrixIndex();
                }
            }
            if ( i + 1 > MaxWeightUsed )
            {
                MaxWeightUsed = i + 1;
            }
        }
    }

    osg::Vec3Array *coords = new osg::Vec3Array;
    coords->reserve(Vertices.size());

    osg::Vec3Array *normals = new osg::Vec3Array;
    normals->reserve(Vertices.size());

    osg::Vec2Array *txt_coords = new osg::Vec2Array;
    txt_coords->reserve(Vertices.size());
    osg::Vec4ubArray *matrix_weights = new osg::Vec4ubArray;
    matrix_weights->reserve(Vertices.size());

    osg::Vec4ubArray *matrix_indices = new osg::Vec4ubArray;
    matrix_indices->reserve(Vertices.size());


    for ( unsigned int i = 0; i < Vertices.size(); i++ )
    {
        coords->push_back ( Vertices [ i ] );
        if ( Normals.size () == Vertices.size() )
        {
            normals->push_back ( Normals [ i ] );
        }
        if ( TxtCoord.size () == Vertices.size() )
        {
            txt_coords->push_back ( TxtCoord [ i ] );
        }
        matrix_weights->push_back ( Matrix_Weights [ i ] );
        matrix_indices->push_back ( Matrix_Indices [ i ] );
    }


    osg::Geometry *geom = new osg::Geometry;

    geom->setVertexArray(coords); 
    if ( Normals.size () == Vertices.size() )
    { 
        geom->setNormalArray(normals); 
        geom->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
    }
    if ( TxtCoord.size () == Vertices.size() )
    {
        geom->setTexCoordArray(0, txt_coords);
    }
    geom->setVertexAttribArray(1,matrix_weights);
    geom->setVertexAttribBinding(1,osg::Geometry::BIND_PER_VERTEX);
    geom->setVertexAttribNormalize(1, GL_TRUE );

    geom->setVertexAttribArray(5,matrix_indices);
    geom->setVertexAttribBinding(5,osg::Geometry::BIND_PER_VERTEX);
    geom->setVertexAttribNormalize(5, GL_FALSE );

    geom->addPrimitiveSet ( new osg::DrawElementsUInt ( osg::PrimitiveSet::TRIANGLES, Indices.size(), (GLuint*)&Indices [ 0 ] ) );
    geom->setCullCallback ( new NamedDrawableCullCallback );
    geom->setComputeBoundingBoxCallback ( new AnimatedDrawableComputeBoundingBoxCallback );
    geom->setName ( Name ); 

    addDrawable ( geom );

    if ( TextureName != "" && !m_bTextureDefined )
    {
        m_bTextureDefined = true;
        
        osg::StateSet* stateset = getOrCreateStateSet();
        
        stateset->setTextureAttributeAndModes( 0, new osg::Texture2D( m_ResourceLoader->getImage ( TextureName )));
        stateset->addUniform( new osg::Uniform("PersonTexture", 0 ) );
    }
}

void AnimationPrototype::BeginAnimationSet( const std::string& AnimationSetName )
{
    if ( m_bNeedToOptimize )
    {   
        // After all the vertices weights are defined we could optimize the bones tree
        // Use the first BeginAnimationSet as the place for the bones tree optimization
        OptimizeBonesTree ( );
        m_bNeedToOptimize = false;
    }
    m_CurAnimationSetForLoad = m_Animations.insert ( std::pair <std::string,AnimationSet>( AnimationSetName, AnimationSet() ) ).first;
    m_CurAnimationSetForLoad -> second.m_LengthInFrames = 0;
}

void AnimationPrototype::BeginAnimation( const std::string& BoneName, int FramesNum )
{
    if ( m_CurAnimationSetForLoad == m_Animations.end() ) // BeginAnimation is called not inside BeginAnimationSet EndAnimationSet pair
    {
        return;
    }

    m_CurAnimationSetForLoad -> second.push_back ( AnimationKey ( ) );
    AnimationKey& ak = m_CurAnimationSetForLoad -> second.back();
    ak.m_pBone = 0;
    ak.reserve( FramesNum );

    m_InitialMatrix.makeIdentity();
    m_bThisMatrixIsNotUsed = false;
    m_bAllEqualToInitial = true;    

    std::map<std::string, Bone *>::iterator It = m_BoneMap.find ( BoneName );
    if ( It != m_BoneMap.end ( ) )
    { 
        m_InitialMatrix = It -> second -> GetInitialMatrix();
        ak.m_pBone = It -> second;
    }
    else
    {
        m_bThisMatrixIsNotUsed = true; // No such a matrix at all or it was removed due to optimization
    }
}

void AnimationPrototype::AddFrame( int Frame, const osg::Matrixf& osgM )
{
    if ( m_CurAnimationSetForLoad -> second.empty() ) // AddFrame is called not inside BeginAnimation EndAnimation pair
    {   
        return;
    }
    if ( m_CurAnimationSetForLoad -> second.m_LengthInFrames < Frame )
    {
        m_CurAnimationSetForLoad -> second.m_LengthInFrames = Frame;
    }
    if ( !IsEqual ( 0.01f, m_InitialMatrix, osgM ) )
    {
        m_bAllEqualToInitial = false;
    }
    AnimationKey& ak = m_CurAnimationSetForLoad -> second.back();
    ak.push_back ( FrameBonePosition ( Frame, osgM.getRotate(), osgM.getTrans () ) );
}

void AnimationPrototype::EndAnimation()
{
    if ( m_CurAnimationSetForLoad -> second.empty() ) // EndAnimation is called without any BeginAnimation before it
    {   
        return;
    }
    if ( m_bAllEqualToInitial || m_bThisMatrixIsNotUsed || m_CurAnimationSetForLoad -> second.back().empty() )
    {
        m_CurAnimationSetForLoad -> second.pop_back();
    }
}

void AnimationPrototype::EndAnimationSet()
{
    if ( m_CurAnimationSetForLoad -> second.empty() )
    {
        m_Animations.erase( m_CurAnimationSetForLoad );
    }
    m_CurAnimationSetForLoad = m_Animations.end();
}

//

} // namespace avAnimation 