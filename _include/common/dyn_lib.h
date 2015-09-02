#pragma once

// based on: http://gcc.gnu.org/wiki/Visibility

////////////////////////////////////////////////////////
// Generic helper definitions for shared library support
#if defined _WIN32 || defined __CYGWIN__
  //#define __HELPER_DL_IMPORT __declspec(dllimport)
  //#define __HELPER_DL_EXPORT __declspec(dllexport)  
  #define __HELPER_DL_IMPORT
  #define __HELPER_DL_EXPORT
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




