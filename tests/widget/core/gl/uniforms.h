#pragma once

namespace core
{

struct base_uniform 
{
protected:
    base_uniform( core::program_ptr program, const char * name )
        : program(program)
    {
        location = glGetUniformLocation(*program, name);
    }

    core::program_ptr program;
    GLint location;
};

template <typename type>
class uniform
{
};

#define UNIFORM_CTOR \
public:                                                 \
    uniform( core::program_ptr program, const char * name ) \
        : base_uniform(program, name)                   \
    {}


#define UNIFORM_1(TYPE, EXT) \
template <>                                                               \
class uniform<TYPE> : public base_uniform                                 \
{                                                                         \
    UNIFORM_CTOR                                                          \
    void set( TYPE value, GLsizei count = 1 )                             \
    {                                                                     \
        glProgramUniform1 ## EXT ## v(*program, location, count, &value); \
    }                                                                     \
};

UNIFORM_1(float, f);
UNIFORM_1(int, i);
UNIFORM_1(unsigned int, ui);

#define UNIFORM_VEC(EXT, COUNT) \
template <>                                                                          \
class uniform<cg::point_ ## COUNT ## EXT> : public base_uniform                     \
{                                                                                    \
    UNIFORM_CTOR                                                                     \
    void set( cg::point_ ## COUNT ## EXT const & value, GLsizei count = 1 )         \
    {                                                                                \
        glProgramUniform ## COUNT ## EXT ## v (*program, location, count, &value.x); \
    }                                                                                \
};

UNIFORM_VEC(f, 2)
UNIFORM_VEC(f, 3)
UNIFORM_VEC(f, 4)

UNIFORM_VEC(i, 2)
UNIFORM_VEC(i, 3)
UNIFORM_VEC(i, 4)

UNIFORM_VEC(ui, 2)
UNIFORM_VEC(ui, 3)
UNIFORM_VEC(ui, 4)

#define UNIFORM_MAT(EXT)                                                                                 \
    template <>                                                                                          \
class uniform<cg::matrix_ ## EXT ## f> : public base_uniform                                             \
{                                                                                                        \
    UNIFORM_CTOR                                                                                         \
    void set( cg::matrix_ ## EXT ## f const & value, GLboolean transpose = GL_FALSE, GLsizei count = 1 ) \
    {                                                                                                    \
        glProgramUniformMatrix ## EXT ## fv (*program, location, transpose, count, value.rawdata());     \
    }                                                                                                    \
};

UNIFORM_MAT(3)
UNIFORM_MAT(4)

}
