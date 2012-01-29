// mmf_log.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "mmf_log.h"


// This is an example of an exported variable
MMF_LOG_API int nmmf_log=0;

// This is an example of an exported function.
MMF_LOG_API int fnmmf_log(void)
{
	return 42;
}

