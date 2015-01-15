#pragma once

// based on: http://gcc.gnu.org/wiki/Visibility

////////////////////////////////////////////////////////
// Generic helper definitions for shared library support
#if defined _WIN32 || defined __CYGWIN__
  #define __HELPER_DL_IMPORT __declspec(dllimport)
  #define __HELPER_DL_EXPORT __declspec(dllexport)
  #define __HELPER_DL_LOCAL
  #pragma warning (disable:4251)
#else
  #if __GNUC__ >= 4
    #define __HELPER_DL_IMPORT
    #define __HELPER_DL_EXPORT __attribute__((used, visibility ("default")))
    #define __HELPER_DL_LOCAL  __attribute__ ((visibility ("hidden")))
  #else
    #define __HELPER_DL_IMPORT
    #define __HELPER_DL_EXPORT
    #define __HELPER_DL_LOCAL
  #endif
#endif


//////////////////////////
// Macroses used in a code

// use this macro for private functions of exported class when you want to preserve them from beeing added to libs export-table
#define API_HDN __HELPER_DL_LOCAL

// ATTENTION: define macroses for your lib like here
// NOTE: macros <PROJECT>_LIB will be defined automatically for your project

/*
#ifdef <PROJECT>_LIB
# define <PROJECT>_API __HELPER_DL_EXPORT
#else
# define <PROJECT>_API __HELPER_DL_IMPORT
#endif
*/

//////////
// sample:

/*
//functions:

<PROJECT>_API void my_func();

void my_func()
{
}

// classes
struct <PROJECT>_API my_class
{
    my_class();

private:
    API_HDN void my_private_class();
}

*//////////
