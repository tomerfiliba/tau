// g_log.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "g_log.h"


// This is an example of an exported variable
G_LOG_API int ng_log=0;

// This is an example of an exported function.
G_LOG_API int fng_log(void)
{
	return 42;
}

// This is the constructor of a class that has been exported.
// see g_log.h for the class definition
Cg_log::Cg_log()
{
	return;
}
