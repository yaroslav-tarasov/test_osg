// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the AV_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// AV_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef AV_EXPORTS
#define AV_API __declspec(dllexport)
#else
#define AV_API __declspec(dllimport)
#endif

// This class is exported from the av.dll
class AV_API Cav {
public:
	Cav(void);
	// TODO: add your methods here.
};

extern AV_API int nav;

AV_API int fnav(void);
