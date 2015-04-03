#pragma once 

#include "cpp_utils/func_pointer.h"

namespace fn_reg
{

    typedef
        boost::unordered_map<std::string, boost::any>
        func_collection_t;

    inline func_collection_t& func_collection()
    {
        static func_collection_t collection;
        return collection;
    }

    //! получение указателя на синглтон коллекции функций
    inline func_collection_t const* extract_collection()
    {
        return &(func_collection());
    }

    //! глобальный регистратор; в конструкторе обращение к синглтону в котором регистрируем объект
    struct func_registrator
    {
        template<typename function_type>
        func_registrator(const char* name, function_type func)
        {
            func_collection()[name] = func;
        }
    };


    inline  boost::any extract_function(std::string const& function_name)
    {
        func_collection_t const& col = func_collection();
        auto it = col.find(function_name);

        if (it == col.end())
            return boost::any();

        return it->second;
    }

    template<typename signature>
    typename cpp_utils::func_pointer<signature>::type
        function( std::string const& function_name)
    {
        // FIXME TODO
        // DECL_LOGGER("nfi");

        boost::any func = extract_function( function_name);

        if (func.empty())
        {
            return nullptr;
        }

        try
        {
            return boost::any_cast<typename cpp_utils::func_pointer<signature>::type>(func);
        }
        catch(boost::bad_any_cast const&)
        {
            // FIXME TODO
            // LogError("function type mismatch: " << function_name << " in " << lib_name);
        }

        return nullptr;
    }

}

#define AUTO_REG_NAME_IMPL(name, func)    \
    namespace                             \
{                                         \
    fn_reg::func_registrator              \
    __registrator__##name(#name, &func);  \
}

#define AUTO_REG_NAME(name, func)   AUTO_REG_NAME_IMPL(name, func)
#define AUTO_REG(func)              AUTO_REG_NAME_IMPL(func, func)
