// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the G_LOG_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// G_LOG_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef G_LOG_EXPORTS
#define G_LOG_API __declspec(dllexport)
#else
#define G_LOG_API __declspec(dllimport)
#endif

// This class is exported from the g_log.dll
class G_LOG_API Cg_log {
public:
	Cg_log(void);
	// TODO: add your methods here.
};

extern G_LOG_API int ng_log;

G_LOG_API int fng_log(void);
