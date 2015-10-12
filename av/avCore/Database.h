#pragma once

namespace Database
{
    std::string LoadShader   (const std::string& name);
    void        initDataPaths();

} // ns Database

namespace avCore
{

	class Database 
	{
		typedef class ShaderObject;

	public:

		enum PathType
		{
			PATH_MODEL,
			PATH_TEXTURE,
			PATH_SHADER
		};

		enum Optimize
		{
			ENABLE_OPTIMIZATION,
			DISABLE_OPTIMIZATION
		};

		class ShaderDefine
		{
		public:
			ShaderDefine() {}
			ShaderDefine( const char* szName, const char* szValue ) : cName( szName ), cValue( szValue ) {}

			std::string         GetDefine() const;

			std::string         cName;
			std::string         cValue;

			static std::string  GetAsString( std::vector< ShaderDefine >& cDefinesVector );
		};
		typedef std::vector< ShaderDefine > ShaderDefineVector;
#define SHADER_DEFINE( vector, name, val )	{ std::stringstream cName, cVal; cName << name; cVal << val; vector.push_back( Database::ShaderDefine( cName.str().c_str(), cVal.str().c_str() ) ); }

	public:

		static Database*        GetInstance();
		static void             Create();
		static void             Release();

		osg::Image*             LoadImage( const char* szFileName );
		osg::Texture2D*         LoadTexture( const char* szFileName, osg::Texture::WrapMode eWrapMode = osg::Texture::REPEAT, bool bUnrefAfterApply = true );
		osg::Shader*            LoadShader( const char* szFileName, ShaderDefineVector* cShaderDefines = NULL, osg::Shader::Type eType = osg::Shader::UNDEFINED );
		osg::Program*           LoadProgram( const char* szVSFileName, const char* szFSFileName, ShaderDefineVector* cShaderDefines = NULL, const char* szGSFileName = NULL );

		// node preprocessor callback
		struct NodePreprocessor : public osg::NodeVisitor
		{
			// constructor
			NodePreprocessor() : osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN) { setNodeMaskOverride(0xFFFFFFFF); }
			// called after applying
			virtual void finish() {}
		};

		osg::Node*              LoadModel( const char* szFileName, const std::vector< std::string >* pDynamicNodes = NULL, NodePreprocessor * pPreprocess = NULL, Optimize eOptimize = ENABLE_OPTIMIZATION );

		bool                    FindFile( PathType ePathType, const std::string& cFileName, std::string* pFullPath = NULL ) const;
		__time64_t              GetFileModificationTime( const char* szFullFileName ) const;

		virtual bool            PreUpdate();

	private:

		Database();
		bool                    LoadShaderInternal( const std::string & fileName, std::ostream & text );

	private:
		class ShaderObject
		{
		public:
			osg::ref_ptr<osg::Shader>   cShaderPtr;
			__time64_t                  nFileModificationTime;
			std::string                 cFileName;
			std::string                 cFullFilePath;
		};

		class ProgramDesc
		{
		public:
			std::string                 cVSFileName;
			std::string                 cGSFileName;
			std::string                 cFSFileName;
			ShaderDefineVector          cDefinesVector;
			osg::ref_ptr<osg::Program>  spProgram;
		};

		typedef std::pair<std::string, std::string>                     ShaderNameAndDefines;
		typedef std::map<std::string, osg::ref_ptr<osg::Node> >         ModelCache;
		typedef std::map<ShaderNameAndDefines, ShaderObject >           ShaderCache;
		typedef std::map<std::string, ProgramDesc >                     ProgramCache;
		typedef std::map<std::string, osg::ref_ptr<osg::Image> >        ImageCache;
		typedef std::map<std::string, osg::ref_ptr<osg::Texture2D> >    TextureCache;

		static Database*        s_pDatabase;
		ModelCache              m_cModelCacheMap;
		ShaderCache             m_cShaderCacheMap;
		ProgramCache            m_cProgramCacheMap;
		ImageCache              m_cImageCacheMap;
		TextureCache            m_cTextureCacheMap;
	};

#define GetDatabase() Database::GetInstance()

}