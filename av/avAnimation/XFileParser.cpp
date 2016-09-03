#include <osg/MatrixTransform>
#include <osgDB/FileUtils>

//#include <avDebug.h>

#include "XFileParser.h"

#include <fstream>
#include <strstream>

namespace avAnimation 
{

bool IsEqual ( float delta, const osg::Matrixf &A, const osg::Matrixf &B );

void RemoveQuotations ( std::string &str )
{
    std::string::size_type pos = str.find('"');
    std::string::size_type end = str.rfind('"');
    if (pos != std::string::npos && end  != std::string::npos ) 
    {
        int len = end-pos-1;
        if ( len < 0 )
        {
            len = 0;
        }
        str = str.substr(pos+1, len);
    }
}

Parser::Parser() :
 m_pStream ( NULL ),
 m_nLineNum ( -1 ),
 m_pLoader ( NULL )
{}

Parser::~Parser()
{}

bool Parser::IsEmpty()
{
    while ( m_Tokens.empty() )
    {                                                        
        char line[1024];
        if ( ! m_pStream->getline ( line, 1024 ).good ())
            return true;
        
        m_nLineNum++; 
        char *Comments = strstr ( line, "//" );
        if ( Comments )
        {
            *Comments = 0; // Cut off comments
        }
        const char *start = line;
        while ( *start )
        {   
            while ( *start == ' ' || *start == '\t' || *start == '\n' || *start == '\r' || *start == ';' || *start == ',' )
            {
                start++;
            }
            if ( *start == 0 )
            {
                break;
            }
            const char *end = start + 1;
            if ( *start != '{' && *start != '}' ) // one symbol tokens that are delimiters itself
            {
                while ( *end != '}' && *end != ';' && *end != ',' && *end != ' ' && *end != '\t' && *end != '\n' && *end != '\r' && *end != 0 )
                {   
                    end++;
                }
            }
            m_Tokens.push_back ( std::string ( start, end ) );
            start = end; 
        }
    } 
    return false;
}

bool Parser::GetNextToken ( std::string &token )
{
    if ( IsEmpty ())
    {
        return false;
    }
    token = m_Tokens.front();
    m_Tokens.pop_front();
    return true;
}

void Parser::GetOutOfScope ( int initial_scope_counter )
{
    std::string token;
    while ( GetNextToken ( token ) )
    {
        if ( token == "{" )
        {
            initial_scope_counter++;
        }
        else if ( token == "}" )
        { 
            if ( --initial_scope_counter == 0 )
            {
                return;
            }
        }
    }
}

void Parser::Error ( const char *Text )
{
    char LineNumberBuffer [10];
    _itoa ( m_nLineNum, LineNumberBuffer, 10 ); 

    std::string Message ( std::string ( "Xfile format error at line: " ) + LineNumberBuffer + " " + Text + "\n" );
    
    // avLog( MT_INFO, Message.c_str() );
}

void Parser::GetRootFrame()
{
    std::string token;
    GetNextToken ( token );
    if ( token != "RootFrame")
    {
        Error ( "RootFrame expected" );
    }
    bool TreatFrameAsBoneTree = true;
    CheckNext ( "{" );
    while ( GetNextToken ( token ) )
    {
        if ( token == "}" )
        {
            break;
        }
        else if ( token == "FrameTransformMatrix" )
        {
            GetOutOfScope ( 0 ); // Just skip this Root matrix
        }
        else if ( token == "Frame" )
        {   
            if ( TreatFrameAsBoneTree )
            { 
                GetBoneFrame();
                TreatFrameAsBoneTree = false;
            }
            else 
            {
                GetFrame();
            }
        }
    }
}

void Parser::GetBoneFrame()
{
    std::string token;
    GetNextToken( token );
    m_pLoader -> AddBone( token );
    m_pLoader -> MoveLevelDown();
    CheckNext ( "{" );
    while ( GetNextToken ( token ) )
    {
        if ( token == "}" )
        {
            break;
        }
        else if ( token == "FrameTransformMatrix" )
        {
            CheckNext ( "{" );
            m_pLoader -> SetBoneInitialMatrix( GetMatrix() );
            CheckNext ( "}" );
        }
        else if ( token == "Frame" )
        {
            GetBoneFrame();
        }
    }
    m_pLoader -> MoveLevelUp();
}


void Parser::GetFrame()
{
    std::string name;
    GetNextToken ( name );
    std::string token;
    CheckNext ( "{" );
    while ( GetNextToken ( token ) )
    {
        if ( token == "}" )
        {
            break;
        }
        else if ( token == "FrameTransformMatrix" )
        {
            CheckNext ( "{" );
            osg::Matrixf osgM ( GetMatrix() );
            static const osg::Matrixf Identity;
            if ( !IsEqual ( 0.01f, osgM, Identity ) )
                Error ( "It's assumed that for the simplification the animated meshes have an identity matrices" );

            CheckNext ( "}" );
        }
        else if ( token == "Frame" )
        {   
            Error ( "It's assumed that nested Frames are not used for animated meshes" );
        }
        else if ( token == "Mesh" )
        {   
            GetMesh ( name );
        }
            
    }
}

void Parser::GetMesh ( const std::string &name )
{
    CheckNext ( "{" );
    int VertexNum = GetInt();
    std::vector<osg::Vec3f> Vertices;
    Vertices.reserve( VertexNum );
    std::vector<osg::Vec4ub> Matrix_Weights( VertexNum );
    std::vector<osg::Vec4ub> Matrix_Indices( VertexNum );
    for ( int i = 0; i < VertexNum; i++ )
    {
        float x = GetFloat();
        float y = GetFloat();
        float z = GetFloat();
        Vertices.push_back ( osg::Vec3f ( x, y, z ) );
    }
    int FacesNum = GetInt();
    std::vector<int> Indices;
    for ( int i = 0; i < FacesNum; i++ )
    {
        int VerticesInFace = GetInt();
        if ( VerticesInFace != 3 && VerticesInFace != 4 )
        {
            Error ( "Must be 3 or 4 vertices in the face" );
        }

        int I0 = GetInt();
        int I1 = GetInt();
        int I2 = GetInt();
        Indices.push_back( I0 );
        Indices.push_back( I1 );
        Indices.push_back( I2 );

        if ( VerticesInFace == 4 )
        {
            int I3 = GetInt();
            Indices.push_back( I0 );
            Indices.push_back( I2 );
            Indices.push_back( I3 );
        }

    }

    std::vector<osg::Vec3f> Normals;
    std::vector<osg::Vec2f> TxtCoord;
    std::string token;
    std::string TextureName; 
    int nBones = 0;
    std::map<std::string, std::vector<std::pair<int,float>>> BonesMapOfVerticesWeghtsTables;
    
    while ( GetNextToken( token ) )
    {
        if ( token == "}" )
        {
            break;
        }
        else if ( token == "MeshMaterialList" )
        {
            GetMeshMaterialList ( TextureName );
        }
        else if ( token == "MeshNormals" )
        {
            CheckNext( "{" );
            int NrmNum = GetInt();
            Normals.reserve( NrmNum );
            for ( int i = 0; i < NrmNum; i++ )
            {
                float nx = GetFloat();
                float ny = GetFloat();
                float nz = GetFloat();
                Normals.push_back( osg::Vec3f ( nx, ny, nz ) );
            }
            // At the moment just skip the normals faces description believing its the same as for vertices
            int FacesNum = GetInt();
            for ( int i = 0; i < FacesNum; i++ )
            {
                int VerticesInFace = GetInt();
                if ( VerticesInFace != 3 && VerticesInFace != 4 )
                {   
                    Error ( "Must be 3 or 4 vertices in the face" );
                }
                GetInt();
                GetInt();
                GetInt();
                if ( VerticesInFace == 4 )
                { 
                    GetInt();
                }
            }
            CheckNext ( "}" );
        }
        else if ( token == "MeshTextureCoords" )
        {
            CheckNext ( "{" );
            int TCNum = GetInt();
            if ( TCNum != VertexNum )
            {
                Error ( "TCNum and VertexNum Must be equal" );
            }
            TxtCoord.reserve(TCNum);
            for ( int i = 0; i < TCNum; i++ )
            {
                float u = GetFloat();
                float v = -GetFloat();
                TxtCoord.push_back ( osg::Vec2f ( u, v ) );
            }
            CheckNext ( "}" );
        }
        else if ( token == "XSkinMeshHeader" )
        {
            CheckNext ( "{" );
            GetInt(); // nMaxSkinWeightsPerVertex 
            GetInt(); // nMaxSkinWeightsPerFace 
            nBones = GetInt();
            CheckNext ( "}" );
        }
        else if ( token == "SkinWeights" )
        {
            CheckNext ( "{" );
            GetNextToken ( token ); // Bone name
            RemoveQuotations ( token );

            std::vector<std::pair<int,float>> &table = BonesMapOfVerticesWeghtsTables [ token ];
            
            int WeightsNumber = GetInt(); 
            std::vector<int> VertexIndices;
            VertexIndices.reserve ( WeightsNumber );
            for ( int i = 0; i < WeightsNumber; i++ )
            {
                VertexIndices.push_back ( GetInt() ); 
            }
            for ( int i = 0; i < WeightsNumber; i++ )
            {
                float weight = GetFloat();
                // store only weights > 0.0
                if ( weight > 0.0 )
                    table.push_back ( std::pair<int,float> ( VertexIndices [ i ], weight ) );
            }
            osg::Matrixf  M ( GetMatrix() );
            if ( !table.empty() )
            {
                m_pLoader -> SetIndexAndModelToBoneMatrix( token, M );
            }
            CheckNext ( "}" );
        }
    }
    m_pLoader->AddDrawable( name,Vertices, Normals, TxtCoord, Indices, BonesMapOfVerticesWeghtsTables, TextureName );
}

void Parser::GetMeshMaterialList ( std::string &TextureName )
{
    // Take only last texture name from this section
    // just skip all the rest information

    CheckNext ( "{" );
    GetInt(); // MaterialNum 
    int FacesNum = GetInt();

    for ( int i = 0; i < FacesNum; i++ )
    {
        GetInt(); // MaterialIndexForTheFace 
    }

    std::string token;
    while ( GetNextToken ( token ) )
    {
        if ( token == "}" )
        {
            break;
        }
        else if ( token == "Material" )
        {
            GetNextToken ( token ); // Name
            CheckNext ( "{" );
            for ( int i = 0; i < 11; i++ ) // 11 is the number of material components
            {   
                GetFloat();
            }
            while ( GetNextToken ( token ) )
            {
                if ( token == "}" )
                {
                    break;
                }
                else if ( token == "TextureFilename" )
                {
                    CheckNext ( "{" );
                    GetNextToken ( token ); // Texture name
                    RemoveQuotations ( token );
                    // do something here with the texture name
                    TextureName = token;
                    CheckNext ( "}" );
                }
            }
        }
    }
}

void Parser::GetAnimationSet()
{
    static int i = 0;
	bool  bflag = false;
    std::string token;
    GetNextToken ( token ); // Name
    
    std::string  ani_set_name = token;
    if ( token == "{" )  // Yeah it's fucking empty, many thx to VM  
    {
        ani_set_name = "default";
		bflag = true;
    }

    m_pLoader -> BeginAnimationSet( ani_set_name );
    if (!bflag) 
		CheckNext ( "{" );
    while ( bflag? true : GetNextToken ( token ) )
    {
        bflag = false;

		if ( token == "}" )
        {
            break;
        }
        else if ( token == "Animation" )
        {
            CheckNext( "{" );
            CheckNext( "{" );
            GetNextToken( token ); // Name
            CheckNext( "}" );
            CheckNext( "AnimationKey" );
            CheckNext( "{" );
            GetInt(); // PositioningMethod == 4
            int FramesNum = GetInt(); 

            m_pLoader -> BeginAnimation( token, FramesNum );
            for ( int f = 0; f < FramesNum; f++ )
            {
                int Frame = GetInt() - 1; // Frames in Blender start from 1
                GetInt(); // ElemInMatrix must be 16
                // Assume the matrix to be not scaled
                m_pLoader -> AddFrame ( Frame, GetMatrix() );
            }
            m_pLoader -> EndAnimation();
            CheckNext( "}" );
            CheckNext( "}" );
        }
    }
    m_pLoader -> EndAnimationSet();
}


void Parser::CheckNext ( const char *Format )
{
    std::string token;
    GetNextToken( token );
    if ( token != Format )
    { 
        std::string Message ( std::string ( "(" ) + token + ") instead of (" + Format + ")" );
        Error ( Message.c_str() );
    }
}

float Parser::GetFloat()
{
    std::string token;
    GetNextToken ( token );
    return atof( token.c_str() );
}

osg::Matrixf Parser::GetMatrix()
{
    float M [ 16 ];
    for ( int i = 0; i < 16; i++ )
    {   
        M [ i ] = GetFloat();
    }
    return osg::Matrixf ( M );
}

float Parser::GetInt()
{
    std::string token;
    GetNextToken( token );
    return atoi( token.c_str() );
}

bool Parser::Load ( std::istream * is, AnimationLoader * Loader )
{
    m_pStream = is ;
  
    m_pLoader = Loader;
    try
    {
        std::string token;
        while ( GetNextToken ( token ) )
        {
            if ( token == "template" )
            {
                GetOutOfScope( 0 );
            }
            else if ( token == "Frame" )
            {
                GetRootFrame();
            }
            else if ( token == "AnimationSet" )
            {
                GetAnimationSet();
            }
        }
    }
    catch ( const std::string &Message )
    {
        osg::notify(osg::FATAL) << Message.c_str();
        return false;
    }
    return true;
}

bool readXFile ( std::istream * is, AnimationLoader * Loader )
{
    Parser parser;
    return parser.Load ( is, Loader );
}

} // namespace avAnimation 