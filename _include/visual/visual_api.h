#pragma once

#ifdef STATIC_VISUAL_API
# define VISUAL_API
#else
#ifdef  VISUAL_EXPORTS
# define VISUAL_API __declspec(dllexport)
#else
# define VISUAL_API __declspec(dllimport)
#endif
#endif


