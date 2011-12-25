// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the MMF_LOG_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// MMF_LOG_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef MMF_LOG_EXPORTS
#define MMF_LOG_API __declspec(dllexport)
#else
#define MMF_LOG_API __declspec(dllimport)
#endif


extern MMF_LOG_API int nmmf_log;

MMF_LOG_API int fnmmf_log(void);
