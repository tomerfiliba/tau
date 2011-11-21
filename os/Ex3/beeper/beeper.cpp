// beeper.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <Windows.h>
#include <assert.h>
#include <math.h>

//keep CPU busy by performing some user-mode instructions (mathematics) and executing some system calls
void DoSomeUserAndKernelWork()
{
	for(unsigned i=1;i<1000;i++)
		{
			for(unsigned j=1;j<1000;j++)
			{
				double b=log(double(i*j));
				DWORD dwTime=GetTickCount();
			}
		}
}


int _tmain(int argc, _TCHAR* argv[])
{
	assert(argc==3);
	DWORD freq=_tstoi(argv[1]);
	DWORD delta=_tstoi(argv[2]);

	while(true)
	{
		//perform beep with given frequency and 500ml (half a second) duration
		BOOL bRes=Beep(freq,500);
		assert(bRes);

		//our function to keep CPU busy
		DoSomeUserAndKernelWork();
		
		//system call to put process into blocking mode for a given time
		Sleep(delta);
	}
	return 0;
}

