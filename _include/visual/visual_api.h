#pragma once

#ifdef NON_DLL
# define VISUAL_API
#else
#ifdef  VISUAL_EXPORTS
# define VISUAL_API __declspec(dllexport)
#else
# define VISUAL_API __declspec(dllimport)
#endif
#endif


