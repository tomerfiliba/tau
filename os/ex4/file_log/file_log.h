// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the FILE_LOG_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// FILE_LOG_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef FILE_LOG_EXPORTS
#define FILE_LOG_API __declspec(dllexport)
#else
#define FILE_LOG_API __declspec(dllimport)
#endif

// This class is exported from the file_log.dll
class FILE_LOG_API Cfile_log {
public:
	Cfile_log(void);
	// TODO: add your methods here.
};

extern FILE_LOG_API int nfile_log;

FILE_LOG_API int fnfile_log(void);
