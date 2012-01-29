// file_log.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "file_log.h"


// This is an example of an exported variable
FILE_LOG_API int nfile_log=0;

// This is an example of an exported function.
FILE_LOG_API int fnfile_log(void)
{
	return 42;
}

// This is the constructor of a class that has been exported.
// see file_log.h for the class definition
Cfile_log::Cfile_log()
{
	return;
}
