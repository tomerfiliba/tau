// ex3.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <windows.h>
#include <conio.h>
#include <assert.h>



BOOL PrintProcessTimes(LPCTSTR proc_name, FILETIME createTime,FILETIME exitTime,FILETIME userTime,FILETIME kernelTime)
{
	ULARGE_INTEGER uCreateTime={createTime.dwLowDateTime,createTime.dwHighDateTime};
	ULARGE_INTEGER uExitTime={exitTime.dwLowDateTime,exitTime.dwHighDateTime};
	ULARGE_INTEGER uUserTime={userTime.dwLowDateTime,userTime.dwHighDateTime};
	ULARGE_INTEGER uKernelTime={kernelTime.dwLowDateTime,kernelTime.dwHighDateTime};

	__int64 runTime=uExitTime.QuadPart-uCreateTime.QuadPart;
	__int64 cpuTime=uUserTime.QuadPart+uKernelTime.QuadPart;

	_tprintf(_T("%s has run time of %d ms. and cpu time of %d ms.\n"),proc_name,DWORD(runTime/10000),DWORD(cpuTime/10000));

	return TRUE;
}


int _tmain(int argc, _TCHAR* argv[])
{
	//prepare command line arguments
	//note that command line is just a way to pass parameters
	//beeper.exe expect firt parameter to be executable path or name
	TCHAR cmd_line1[MAX_PATH],cmd_line2[MAX_PATH];
	_tcscpy_s(cmd_line1,_T("beeper.exe 500 1000"));
	_tcscpy_s(cmd_line2,_T("beeper.exe 1000 2000"));

	//prepare startup info (e.g. size of the window). Use system defaults
	STARTUPINFO si1,si2;
	ZeroMemory(&si1,sizeof(si1));
	ZeroMemory(&si2,sizeof(si2));
	si1.cb=si2.cb=sizeof(si1); //have to put size of the structure for compatibility

	//output strucuture , will be filled with process  handle and other parameters
	PROCESS_INFORMATION pi1,pi2;
	ZeroMemory(&pi1,sizeof(pi1));
	ZeroMemory(&pi2,sizeof(pi2));

	//create processes 
	BOOL bRes1=CreateProcess(_T("C:\\Users\\sasha\\Desktop\\Teaching\\Code\\ex3\\debug\\beeper.exe"),cmd_line1, NULL,NULL,FALSE,CREATE_NO_WINDOW,NULL,NULL, &si1,&pi1); 
	BOOL bRes2=CreateProcess(_T("C:\\Users\\sasha\\Desktop\\Teaching\\Code\\ex3\\debug\\beeper.exe"),cmd_line2, NULL,NULL,FALSE,CREATE_NO_WINDOW,NULL,NULL, &si2,&pi2); 
	assert(bRes1 && bRes2);
	
	_tprintf(_T("processes are started, press any key to stop\n"));
	_getch();

	//kill processes, not very nice 
	BOOL bTermRes1=TerminateProcess(pi1.hProcess,0);
	BOOL bTermRes2=TerminateProcess(pi2.hProcess,0);
	assert(bTermRes1 && bTermRes2);

	_tprintf(_T("processes are terminated, press any key to show times\n"));
	_getch();

	//get process statistics
	FILETIME createTime1, createTime2, exitTime1, exitTime2,kernelTime1,kernelTime2,userTime1,userTime2;
	BOOL bTimeRes1=GetProcessTimes(pi1.hProcess,&createTime1,&exitTime1,&kernelTime1,&userTime1);
	BOOL bTimeRes2=GetProcessTimes(pi2.hProcess,&createTime2,&exitTime2,&kernelTime2,&userTime2);
	assert(bTimeRes1 && bTimeRes2);

	//close all handles
	CloseHandle(pi1.hProcess);
	CloseHandle(pi2.hProcess);
	CloseHandle(pi1.hThread);
	CloseHandle(pi2.hThread);

	//print process times
	BOOL bPrintRes1=PrintProcessTimes(_T("process A"), createTime1, exitTime1, kernelTime1,userTime1);
	BOOL bPrintRes2=PrintProcessTimes(_T("process B"), createTime2, exitTime2, kernelTime2,userTime2);

	_tprintf(_T("processes are terminated, press any key to exit\n"));
	_getch();

	return 0;
}

