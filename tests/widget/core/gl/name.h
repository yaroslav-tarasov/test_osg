#pragma once

namespace core 
{

struct Named
    : context_object
    , public ref_counter
{
    operator GLuint const & () const
    {
        return id;
    }

protected:
    GLuint id;
};

#define NAMED(Name, GLName)                     \
    struct Name : public Named                  \
    {                                           \
        Name() {                                \
            ASSERT_CONTEXT_CURRENT              \
            glGen ## GLName ## s (1, &id);      \
        }                                       \
                                                \
        ~Name() {                               \
            MAKE_CONTEXT_CURRENT                \
            glDelete ## GLName ## s (1, &id);   \
        }                                       \
    };                                          \
    typedef boost::intrusive_ptr<Name> Name ## _ptr;

NAMED(texture, Texture)
NAMED(buffer, Buffer)
NAMED(vertex_array, VertexArray)
NAMED(framebuffer, Framebuffer)
NAMED(pipeline, ProgramPipeline)
NAMED(query, Querie)
NAMED(transform_feedback, TransformFeedback)

//////////////////////////////////////////////////////////////////////////

#define GET_SHADER_BIT(SHADER_TYPE) \
    SHADER_TYPE == GL_VERTEX_SHADER ? GL_VERTEX_SHADER_BIT : \
    SHADER_TYPE == GL_GEOMETRY_SHADER ? GL_GEOMETRY_SHADER_BIT : \
    GL_FRAGMENT_SHADER_BIT

struct shader : public Named
{
    const GLenum type;

    shader( GLenum type )
        : type(type)
    {
    }
};
typedef boost::intrusive_ptr<shader> shader_ptr;

template <GLenum TYPE>
struct concrete_shader : public shader
{
    concrete_shader( const char * shader_source )
        : shader(TYPE)
    {
        id = glCreateShader(TYPE);
        if (id == 0)
            LogError("glCreateShader fail : probably context not bound!");
        glShaderSource(id, 1, &shader_source, nullptr);
        glCompileShader(id);
        checkShader(shader_source);
    }

    ~concrete_shader()
    {
        MAKE_CONTEXT_CURRENT
        glDeleteShader(id);
    }

private:

    void checkShader( const char * shader_source )
    {
        GLint Result = GL_FALSE;
        glGetShaderiv(id, GL_COMPILE_STATUS, &Result);

        if (Result == GL_FALSE) {
            std::stringstream ss;
            ss << '\n';

            QString source = QString::fromLatin1(shader_source);
            source.replace('%', QString("%%"));
            QStringList strings = source.split(QChar('\n'));
            for (int i = 0; i < strings.length(); ++i)
                ss << i + 1 << ") " << strings[i].toStdString().c_str() << "\n";
            ss << "Compiling shader errors:\n";

            int InfoLogLength;
            glGetShaderiv(id, GL_INFO_LOG_LENGTH, &InfoLogLength);
            std::string buf(" ", InfoLogLength + 1);
            glGetShaderInfoLog(id, InfoLogLength, nullptr, &buf[0]);
            ss << buf;

            LogError(ss.str().c_str());
        }
    }
};

typedef concrete_shader<GL_VERTEX_SHADER>   vertex_shader;
typedef boost::intrusive_ptr<vertex_shader> vertex_shader_ptr;

typedef concrete_shader<GL_GEOMETRY_SHADER> geometry_shader;
typedef boost::intrusive_ptr<geometry_shader> geometry_shader_ptr;

typedef concrete_shader<GL_FRAGMENT_SHADER> fragment_shader;
typedef boost::intrusive_ptr<fragment_shader> fragment_shader_ptr;

struct program : public Named
{
    program( std::vector<shader_ptr> & shaders )
    {
        id = glCreateProgram();
        for (size_t i = 0; i < shaders.size(); ++i)
        {
            shaders_.push_back(shaders[i]);
            glAttachShader(id, *shaders[i]);
        }
        glLinkProgram(id);

        finalize();
    }

    program( GLenum cv, std::vector<char> const & bin_cache )
        : cache_ver_(cv)
        , cache_(std::move(bin_cache))
    {
        id = glCreateProgram();
        if (cache_.size())
            glProgramBinary(id, cache_ver_, &cache_.front(), cache_.size());

        finalize(false);
    }

    ~program()
    {
        MAKE_CONTEXT_CURRENT
        glDeleteProgram(id);
    }

public:
    bool status() const
    {
        return result_ == GL_TRUE;
    }

    GLenum get_cache_version() const
    {
        return cache_ver_;
    }
    std::vector<char> const & get_cache_binaries() const
    {
        return cache_;
    }

private:
    void finalize( bool get_bin = true )
    {
        result_ = GL_FALSE;
        glGetProgramiv(id, GL_LINK_STATUS, &result_);

        if (result_ == GL_FALSE)
        {
            std::stringstream ss;
            ss << "Error linking OpenGL program\n";
            int InfoLogLength;
            glGetProgramiv(id, GL_INFO_LOG_LENGTH, &InfoLogLength);
            std::vector<char> Buffer(std::max<int>(InfoLogLength, 1));
            glGetProgramInfoLog(id, InfoLogLength, nullptr, &Buffer[0]);
            ss << &Buffer[0];
            LogError(ss.str().c_str());
        }
        else if (get_bin)
        {
            GLint binaryLength = 0;
            glGetProgramiv(id, GL_PROGRAM_BINARY_LENGTH, &binaryLength);
            Assert(binaryLength);
            GLint actual_size = 0;
            cache_.resize(binaryLength);
            glGetProgramBinary(id, binaryLength, &actual_size, &cache_ver_, &cache_[0]);
        }
    }

    std::vector<shader_ptr> shaders_;
    GLint result_;

    GLenum cache_ver_;
    std::vector<char> cache_;
};
typedef boost::intrusive_ptr<program> program_ptr;

}
