#include "stdafx.h"

#include <sys/stat.h>

#include "Database.h"
#include "Utils.h"
#include "Timer.h"

namespace Database
{

void initDataPaths()
{
	osgDB::getDataFilePathList().push_back(osgDB::getCurrentWorkingDirectory() + "\\data");
    osgDB::getDataFilePathList().push_back(osgDB::getCurrentWorkingDirectory() + "\\data\\models");
    osgDB::getDataFilePathList().push_back(osgDB::getCurrentWorkingDirectory() + "\\data\\areas");
    osgDB::getDataFilePathList().push_back(osgDB::getCurrentWorkingDirectory() + "\\data\\areas\\misc");
    osgDB::getDataFilePathList().push_back(osgDB::getCurrentWorkingDirectory() + "\\data\\materials");
    osgDB::getDataFilePathList().push_back(osgDB::getCurrentWorkingDirectory() + "\\data\\materials\\sky");
    osgDB::getDataFilePathList().push_back(osgDB::getCurrentWorkingDirectory() + "\\data\\materials\\lib");  
	osgDB::getDataFilePathList().push_back(osgDB::getCurrentWorkingDirectory() + "\\data\\materials\\misc");
	osgDB::getDataFilePathList().push_back(osgDB::getCurrentWorkingDirectory() + "\\data\\images");   

    osgDB::getDataFilePathList().push_back(osgDB::getCurrentWorkingDirectory() + "\\data\\models\\human");
}

bool LoadShaderFileInternal(const std::string & fileName, std::ostream & text);

bool LoadShaderInternal(const std::string & fileName, std::stringstream& file, std::ostream & text )
{
#if 0
    std::ifstream file(fileName.c_str());

    if (file.bad())
    {

        avError(/*_T*/("Failed to load shader '%s'."), fileName.c_str());

        return false;
    }
#endif

    bool commentBlock = false;
    char szLineBuffer[1024];
    while (file.getline(szLineBuffer, sizeof(szLineBuffer)))
    {
        const char * pLineBuffer = szLineBuffer;

        // trim left
        while (*pLineBuffer == ' ')
            pLineBuffer++;

        //
        // Note: Not supported spaces after '#' and before 'include'
        //

        if (!commentBlock && strncmp(pLineBuffer, "#include", 8) == 0)
        {
            pLineBuffer += 8;
            const char * beginFileName = strchr(pLineBuffer, '"') + 1;
            const char * endFileName = strchr(beginFileName, '"');

            if (beginFileName == NULL || endFileName == NULL || beginFileName == endFileName)
            {
                avError(/*_T*/("#include directive error in shader '%s' shader."), fileName.c_str());

                continue;
            }

            const std::string includeFileName(beginFileName, endFileName);
            std::string includeFullFileName = osgDB::findDataFile(includeFileName);
            
            if (!includeFullFileName.empty())
            {
                if (!LoadShaderFileInternal(includeFullFileName, text))
                {
                    avError(/*_T*/("Failed to load shader '%s'."), fileName.c_str());

                    return false;
                }
            }
            else
            {
                avError(/*_T*/("Failed to find shader include '%s'."), includeFileName.c_str());
                avError(/*_T*/("Failed to load shader '%s'."), fileName.c_str());

                return false;
            }
        }
        else
        {
            //
            // Note: Not supported all cases when comment block closes before 
            //       #include directive on same text line.
            // /* some comment
            //    blah-blah-blah
            // */ #include "file.inl"
            //

            const char * pComment = pLineBuffer;
            while (pComment != NULL)
            {
                if (!commentBlock && (pComment = strstr(pComment, "/*")) != NULL)
                    commentBlock = true;
                else if (commentBlock && (pComment = strstr(pComment, "*/")) != NULL)
                    commentBlock = false;
            }

            text << szLineBuffer << std::endl;
        }
    }

    return true;
}

bool LoadShaderFileInternal(const std::string & fileName, std::ostream & shaderText)
{
    const std::string  file_name = osgDB::findDataFile(fileName);
    std::ifstream file(file_name.c_str());
    if (file.bad())
    {
        avError("Failed to load shader '%s'.", fileName.c_str());
        return false;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();

    if (!LoadShaderInternal(fileName, buffer, shaderText))
        return false;

    return true;
}

std::string LoadShader(const std::string& name)
{
    std::ostringstream shaderText;
    //  shaderText << cIt->first.second; // Add all defines

    if (!LoadShaderFileInternal(name, shaderText))
        return false;

    shaderText << std::ends;

    return shaderText.str();
}

} // ns Database



#define SV_USE_MULTITHREADING 1

namespace avCore
{

using namespace Utils;

Database* Database::s_pDatabase = NULL;

static CRITICAL_SECTION DatabaseCS;

//////////////////////////////////////////////////////////////////////////
Database::Database()
{
}

//////////////////////////////////////////////////////////////////////////
Database* Database::GetInstance()
{
    avAssert( s_pDatabase );
    return s_pDatabase;
}

//////////////////////////////////////////////////////////////////////////
void Database::Create()
{
    avAssert( s_pDatabase == NULL );
    s_pDatabase = new Database;
    InitializeCriticalSection ( &DatabaseCS );
}

//////////////////////////////////////////////////////////////////////////
void Database::Release()
{
    DeleteCriticalSection ( &DatabaseCS );
    avAssert( s_pDatabase );
    svReleaseMem( s_pDatabase );
}

//////////////////////////////////////////////////////////////////////////
osg::Image* Database::LoadImage( const char* szFileName )
{
    avAssert( szFileName );

    ImageCache::iterator cIt = m_cImageCacheMap.find( szFileName );
    if ( cIt != m_cImageCacheMap.end() )
    {
        return cIt->second.get();
    }

    osg::Image*	pImage = NULL;
    std::string	cFullFilePath;

    if ( FindFile( PATH_TEXTURE, szFileName, &cFullFilePath ) == true )
    {
        pImage = osgDB::readImageFile( cFullFilePath.c_str(), new osgDB::Options(""));
        if ( pImage )
        {
            pImage->setDataVariance(osg::Object::STATIC);
        }
    }

    if ( pImage == NULL )
    {
        avError( "Failed to load texture '%s'.", szFileName );
    } else {
        m_cImageCacheMap[ szFileName ] = pImage;
    }

    return pImage;
}

//////////////////////////////////////////////////////////////////////////
osg::Texture2D* Database::LoadTexture( const char* szFileName,
                                       osg::Texture::WrapMode eWrapMode /* = osg::Texture::REPEAT */,
                                       bool bUnrefAfterApply /* = true */ )
{
    TextureCache::iterator cIt = m_cTextureCacheMap.find( szFileName );
    if ( cIt != m_cTextureCacheMap.end() )
    {
        return cIt->second.get();
    }

    osg::Image*	pImage = Database::LoadImage( szFileName );
    if ( pImage == NULL )
        return NULL;

    osg::Texture2D* pTexture = new osg::Texture2D();
    pTexture->setWrap( osg::Texture2D::WRAP_S, eWrapMode );
    pTexture->setWrap( osg::Texture2D::WRAP_T, eWrapMode );
    pTexture->setFilter( osg::Texture2D::MIN_FILTER,osg::Texture2D::LINEAR_MIPMAP_LINEAR );
    pTexture->setFilter( osg::Texture2D::MAG_FILTER,osg::Texture2D::LINEAR );
    pTexture->setImage( pImage );
    pTexture->setDataVariance(osg::Object::STATIC);

    pTexture->setUnRefImageDataAfterApply(bUnrefAfterApply);

    m_cTextureCacheMap[ szFileName ] = pTexture;

    return pTexture;
}

//////////////////////////////////////////////////////////////////////////
osg::Shader * Database::LoadShader( const char * szFileName, ShaderDefineVector* cShaderDefines /*= NULL*/, osg::Shader::Type eType /*= osg::Shader::UNDEFINED*/ )
{
    avAssert(szFileName != NULL);
    std::string cFileName( szFileName );

    // Create shader defines
    std::string cDefines;
    if ( cShaderDefines != NULL )
    {
        cDefines = ShaderDefine::GetAsString( *cShaderDefines );
    }

    ShaderNameAndDefines cShaderNameAndDefines;
    cShaderNameAndDefines.first  = cFileName;
    cShaderNameAndDefines.second = cDefines;

    // Find shader in cache
    ShaderCache::iterator cIt = m_cShaderCacheMap.find( cShaderNameAndDefines );
    if (cIt != m_cShaderCacheMap.end())
        return cIt->second.cShaderPtr.get();

    // Auto-detect shader type
    if (eType == osg::Shader::UNDEFINED)
    {
        const size_t dotpos = cFileName.find_last_of('.');
        if (dotpos == std::string::npos)
        {
            avError("Unable to detect shader type, no extension found '%s'.", cFileName.c_str());
            return NULL;
        }
        else
        {
            const std::string extension = cFileName.substr(dotpos + 1);
            if (extension == "vs" || extension == "vert")
                eType = osg::Shader::VERTEX;
            else if (extension == "gs" || extension == "geom")
                eType = osg::Shader::GEOMETRY;
            else if (extension == "fs" || extension == "frag")
                eType = osg::Shader::FRAGMENT;
            else
            {
                avError("Unknown shader type '%s'.", cFileName.c_str());
                return NULL;
            }
        }
    }

    // Load shader
    ShaderObject cShaderObject;
    cShaderObject.cShaderPtr = new osg::Shader(eType);
    cShaderObject.cFileName = cFileName;

    if (FindFile(PATH_SHADER, cFileName.c_str(), &cShaderObject.cFullFilePath))
    {
        cShaderObject.nFileModificationTime = GetFileModificationTime(cShaderObject.cFullFilePath.c_str());

        std::ostringstream shaderText;
        shaderText << cDefines;
        if (!LoadShaderInternal(cShaderObject.cFullFilePath, shaderText))
            return NULL; // error already handled
        
        shaderText << std::ends;
        cShaderObject.cShaderPtr->setName(cShaderObject.cFileName);
        cShaderObject.cShaderPtr->setShaderSource(shaderText.str().c_str());
    }
    else
    {
        avError("Failed to find shader file '%s'.", cFileName.c_str());
        return NULL;
    }

    // Add shader to the cache
    m_cShaderCacheMap[ cShaderNameAndDefines ] = cShaderObject;

    return cShaderObject.cShaderPtr.get();
}

//////////////////////////////////////////////////////////////////////////
osg::Program* Database::LoadProgram( const char* szVSFileName, const char* szFSFileName, ShaderDefineVector* pShaderDefines /*= NULL*/, const char* szGSFileName /*= NULL*/ )
{
    // Create hash string
    std::stringstream cHashString;
    cHashString << ( szVSFileName ? szVSFileName : "no_vs" ) << std::endl;
    cHashString << ( szGSFileName ? szGSFileName : "no_gs" ) << std::endl;
    cHashString << ( szFSFileName ? szFSFileName : "no_fs" ) << std::endl;
    cHashString << ( pShaderDefines ? ShaderDefine::GetAsString( *pShaderDefines ) : "no_def" ) << std::ends;

    // Find program in the cache
    osg::ref_ptr<osg::Program> spProgram;
    ProgramCache::iterator cIt = m_cProgramCacheMap.find( cHashString.str() );
    if ( cIt != m_cProgramCacheMap.end() )
    {
        spProgram = cIt->second.spProgram;
        avAssert( spProgram.valid() );
    }
    else
    {   
        // Vertex shader
        osg::ref_ptr<osg::Shader> spVS;
        if ( szVSFileName != NULL )
        {
            spVS = LoadShader( szVSFileName, pShaderDefines, osg::Shader::VERTEX );
            if ( spVS.valid() == false )
            {
                return NULL;
            }
        }

        // Fragment shader
        osg::ref_ptr<osg::Shader> spFS;
        if ( szFSFileName != NULL )
        {
            spFS = LoadShader( szFSFileName, pShaderDefines, osg::Shader::FRAGMENT );
            if ( spFS.valid() == false )
            {
                return NULL;
            }
        }

        // Geometry shader
        osg::ref_ptr<osg::Shader> spGS;
        if ( szGSFileName != NULL )
        {
            spGS = LoadShader( szGSFileName, pShaderDefines, osg::Shader::GEOMETRY );
            if ( spGS.valid() == false )
            {
                return NULL;
            }
        }

        // Create new program
        spProgram = new osg::Program;
        if ( spVS.valid() ) spProgram->addShader( spVS );
        if ( spFS.valid() ) spProgram->addShader( spFS );
        if ( spGS.valid() ) spProgram->addShader( spGS );

        // Add program to the cache
        ProgramDesc cProgramDesc;
        cProgramDesc.cVSFileName    = ( szVSFileName ? szVSFileName : "" );
        cProgramDesc.cGSFileName    = ( szGSFileName ? szGSFileName : "" );
        cProgramDesc.cFSFileName    = ( szFSFileName ? szFSFileName : "" );
        cProgramDesc.cDefinesVector = ( pShaderDefines ? *pShaderDefines : ShaderDefineVector() );
        cProgramDesc.spProgram      = spProgram;
        m_cProgramCacheMap[ cHashString.str() ] = cProgramDesc;
    }
    
    return spProgram.get();
}

bool Database::LoadShaderInternal( const std::string & fileName, std::ostream & text )
{
    std::ifstream file(fileName.c_str());

    if (file.bad())
    {
        avError("Failed to load shader '%s'.", fileName.c_str());
        return false;
    }

    bool commentBlock = false;
    char szLineBuffer[1024];
    while (file.getline(szLineBuffer, sizeof(szLineBuffer)))
    {
        const char * pLineBuffer = szLineBuffer;

        // trim left
        while (*pLineBuffer == ' ')
            pLineBuffer++;

        //
        // Note: Not supported spaces after '#' and before 'include'
        //

        if (!commentBlock && strncmp(pLineBuffer, "#include", 8) == 0)
        {
            pLineBuffer += 8;
            const char * beginFileName = strchr(pLineBuffer, '"') + 1;
            const char * endFileName = strchr(beginFileName, '"');

            if (beginFileName == NULL || endFileName == NULL || beginFileName == endFileName)
            {
                avError("#include directive error in shader '%s' shader.", fileName.c_str());
                continue;
            }

            const std::string includeFileName(beginFileName, endFileName);
            std::string includeFullFileName;
            if (FindFile(PATH_SHADER, includeFileName.c_str(), &includeFullFileName))
            {
                if (!LoadShaderInternal(includeFullFileName, text))
                {
                    avError("Failed to load shader '%s'.", fileName.c_str());
                    return false;
                }
            }
            else
            {
                avError("Failed to find shader include '%s'.", includeFileName.c_str());
                avError("Failed to load shader '%s'.", fileName.c_str());
                return false;
            }
        }
        else
        {
            //
            // Note: Not supported all cases when comment block closes before 
            //       #include directive on same text line.
            // /* some comment
            //    blah-blah-blah
            // */ #include "file.inl"
            //

            const char * pComment = pLineBuffer;
            while (pComment != NULL)
            {
                if (!commentBlock && (pComment = strstr(pComment, "/*")) != NULL)
                    commentBlock = true;
                else if (commentBlock && (pComment = strstr(pComment, "*/")) != NULL)
                    commentBlock = false;
            }

            text << szLineBuffer << std::endl;
        }
    }

    return true;
}


//////////////////////////////////////////////////////////////////////////
osg::Node* Database::LoadModel( const char* szFileName, const std::vector< std::string >* pDynamicNodes /*= NULL*/,
                                Database::NodePreprocessor * pPreprocess/* = NULL*/, Optimize eOptimize /*= ENABLE_OPTIMIZATION*/ )
{
    avAssert( szFileName );

    osg::Node*  pNode = NULL;
    std::string cFullFilePath;

    if ( FindFile( PATH_MODEL, szFileName, &cFullFilePath ) == true )
    {
#if SV_USE_MULTITHREADING
        // Just change P_o_c_o::Mutex to CriticalSection
        // but this code initially was not completely correct: need to think about simultaneous loading of the same object
        EnterCriticalSection ( &DatabaseCS );
#endif
        osg::ref_ptr<osg::Node>& cNodePtr = m_cModelCacheMap[szFileName];
#if SV_USE_MULTITHREADING
        LeaveCriticalSection( &DatabaseCS );
#endif
        if ( !cNodePtr.valid() )
        {
            // Load from IVE or FLT by using OpenSceneGraph plugin
            cNodePtr = osgDB::readNodeFile( cFullFilePath.c_str() );

            if ( cNodePtr.valid() )
            {
                // model preprocessor
                if (pPreprocess)
                {
                    // apply visitor
                    cNodePtr->accept(*pPreprocess);
                    // finalize visitor work after node
                    pPreprocess->finish();
                }

                NodeFinder cNodeFinder;
                cNodeFinder.SetNode( cNodePtr.get() );
                osgUtil::Optimizer cOptimizer;

                // Disable optimization for selected nodes by marking them as dynamic
                if ( pDynamicNodes != NULL )
                {
                    for ( uint32_t nNodeIndex = 0; nNodeIndex < pDynamicNodes->size(); nNodeIndex++ )
                    {
                        osg::Node* pDynamicNode = cNodeFinder.FindChildByName_nocase( pDynamicNodes->at( nNodeIndex ).c_str() );
                        if ( pDynamicNode )
                        {
                            // Do not optimize selected node
                            cOptimizer.setPermissibleOptimizationsForObject( pDynamicNode, 0 );

                            // Set dynamic node
                            SetDynamicNode( pDynamicNode );
                        }
                    }
                }

                // Remove legacy states
                PostprocessModel( cNodePtr.get() );

                // Optimize only what's necessary
                if ( eOptimize == ENABLE_OPTIMIZATION )
                {
                    cOptimizer.optimize( cNodePtr.get(), 
                        osgUtil::Optimizer::FLATTEN_STATIC_TRANSFORMS |
                        osgUtil::Optimizer::REMOVE_REDUNDANT_NODES |
                        osgUtil::Optimizer::SHARE_DUPLICATE_STATE |
                        osgUtil::Optimizer::MERGE_GEOMETRY |
                        osgUtil::Optimizer::MERGE_GEODES |
                        osgUtil::Optimizer::STATIC_OBJECT_DETECTION );
                }
            }
        }

        // Clone model from a cache
        if ( cNodePtr.valid() )
        {
            static const osg::CopyOp s_cCloneOptions = osg::CopyOp::DEEP_COPY_NODES;
            pNode = safe_cast<osg::Node*>( cNodePtr->clone( s_cCloneOptions ) );
        }
    }

    if ( pNode == NULL )
    {
        avError( "Failed to load model '%s'.", szFileName );
    }

    return pNode;
}

//////////////////////////////////////////////////////////////////////////
bool Database::FindFile( PathType ePathType, const std::string& cFileName, std::string* pFullPath /*= NULL*/ ) const
{
    avAssert( cFileName.empty() == false );

	//fpl_wrap  fpl(cFileName);
	//*pFullPath =  osgDB::findFileInPath(cFileName, fpl.fpl_, osgDB::CASE_INSENSITIVE);
	
	osgDB::FilePathList fpl = osgDB::getDataFilePathList();

	*pFullPath = osgDB::findFileInPath(cFileName, fpl,osgDB::CASE_INSENSITIVE);

	if (pFullPath)
		return true;

#if 0
    // Get number of paths
    uint32_t nNumOfPaths;
    switch ( ePathType )
    {
    case PATH_MODEL:
        nNumOfPaths = avCore::GetAppConfiguration()->GetNumOfModelPaths();
        break;
    case PATH_TEXTURE:
        nNumOfPaths = avCore::GetAppConfiguration()->GetNumOfTexturePaths();
        break;
    case PATH_SHADER:
        nNumOfPaths = avCore::GetAppConfiguration()->GetNumOfShaderPaths();
        break;
    default:
        avError( "Unknown path type" );
    }

    // Search all paths
    for ( uint32_t i = 0; i < nNumOfPaths; i++ )
    {
        std::string cFullFileName;
        switch ( ePathType )
        {
        case PATH_MODEL:
            cFullFileName = avCore::GetAppConfiguration()->GetModelPath( i );
            break;
        case PATH_TEXTURE:
            cFullFileName = avCore::GetAppConfiguration()->GetTexturePath( i );
            break;
        case PATH_SHADER:
            cFullFileName = avCore::GetAppConfiguration()->GetShaderPath( i );
            break;
        default:
            avError( "Unknown path type" );
        }
        cFullFileName += cFileName;

        // Check if a file exists
        if ( avCore::CheckIfAFileExists( cFullFileName ) )
        {
            if ( pFullPath )
            {
                *pFullPath = cFullFileName;
            }
            return true;
        }
    }

    // Try to open file from absolute path
    if ( avCore::CheckIfAFileExists( cFileName ) )
    {
        if ( pFullPath )
        {
            *pFullPath = cFileName;
        }
        return true;
    }
#endif
    // File doesn't exist
    return false;
}

//////////////////////////////////////////////////////////////////////////
__time64_t Database::GetFileModificationTime( const char* szFullFileName ) const
{
    struct __stat64 sStat;
    if ( _stat64( szFullFileName, &sStat ) == 0 )
    {
        return sStat.st_mtime;
    }

    return 0;
}

//////////////////////////////////////////////////////////////////////////
bool Database::PreUpdate()
{
    #define UPDATE_SHADERS_TIME 1.0
    static double s_dbTimeLeftToUpdateShaders = UPDATE_SHADERS_TIME;
    
    s_dbTimeLeftToUpdateShaders -= GetTimer()->GetFrameTimeDelta();
    if ( s_dbTimeLeftToUpdateShaders <= 0.0 )
    {
        s_dbTimeLeftToUpdateShaders = UPDATE_SHADERS_TIME;
        for ( ShaderCache::iterator cIt = m_cShaderCacheMap.begin(); cIt != m_cShaderCacheMap.end(); cIt++ )
        {
            ShaderObject & cShaderObject = cIt->second;
            __time64_t nTime = GetFileModificationTime( cShaderObject.cFullFilePath.c_str() );
            if ( cShaderObject.nFileModificationTime != nTime )
            {
                std::ostringstream shaderText;
                shaderText << cIt->first.second; // Add all defines
                if (!LoadShaderInternal(cShaderObject.cFullFilePath, shaderText))
                    continue;
                shaderText << std::ends;

                cShaderObject.nFileModificationTime = nTime;
                cShaderObject.cShaderPtr->setName(cShaderObject.cFileName);
                cShaderObject.cShaderPtr->setShaderSource(shaderText.str().c_str());
            }
        }
    }

    return true;
}

//////////////////////////////////////////////////////////////////////////
std::string Database::ShaderDefine::GetDefine() const
{
    static std::ostringstream cDefine;
    cDefine.str( "" );
    cDefine << "#define " << cName << " " << cValue << std::endl;
    return cDefine.str();
}

//////////////////////////////////////////////////////////////////////////
std::string Database::ShaderDefine::GetAsString( std::vector< ShaderDefine >& cDefinesVector )
{
    std::string cDefinesString;
    for ( uint32_t nIndex = 0; nIndex < cDefinesVector.size(); nIndex++ )
    {
        cDefinesString += cDefinesVector[ nIndex ].GetDefine();
    }

    return cDefinesString;
}

}