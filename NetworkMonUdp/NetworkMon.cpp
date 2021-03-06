#include <iostream>
#include "osdepend.h"
#include <string>
#include "NmCpMgr.h"
#include "Logger.h"
#include <signal.h>

#ifdef WIN32
#include "NTService.h"
#include "psapi.h"
#pragma  comment(lib,"Psapi")
#endif

CNmCpMgr theMgr;

#ifdef WIN32
BOOL WINAPI ConsoleHandler(DWORD dwEvent)
{
	switch ( dwEvent )
	{
	case CTRL_C_EVENT:
		break;
	case CTRL_BREAK_EVENT:
		break;
	case CTRL_CLOSE_EVENT:
		break;
	case CTRL_LOGOFF_EVENT:
		
		break;
	case CTRL_SHUTDOWN_EVENT:
		break;
	}
	return TRUE;
}

void GetProcessName( DWORD dwProcessID, char* szProcessName, int nLen )
{
    HANDLE hProcess = OpenProcess( PROCESS_QUERY_INFORMATION |
                                   PROCESS_VM_READ,
                                   FALSE, dwProcessID );
    if (NULL != hProcess )
    {
        HMODULE hMod;
        DWORD dwNeeded;

        if ( EnumProcessModules( hProcess, &hMod, sizeof(hMod), 
             &dwNeeded) )
        {
           // GetModuleBaseName( hProcess, hMod, szProcessName,nLen );
            GetModuleFileNameEx( hProcess, hMod, szProcessName,nLen );
		}
    }

    CloseHandle( hProcess );

}
#endif

int main(int argc, char* argv[])
{
	try
	{
		std::string sProcName = "NmMon";
#ifdef WIN32
		char szFileName[_MAX_PATH], szFilePath[_MAX_PATH];
		char * pcName;
		::GetModuleFileName(0,szFileName, _MAX_PATH);
		::GetFullPathName(szFileName, _MAX_PATH, szFilePath, &pcName);
		char szBuf[_MAX_PATH];
		strcpy(szBuf, pcName);
		*pcName = '\0';
		SetCurrentDirectory(szFilePath);
		
		//char * pcTmp = szBuf + strlen(szBuf) - 1;
		//for ( ; pcTmp >= szBuf; pcTmp-- )
		//{
		//	if ( *pcTmp == '.' )
		//	{
		//		*pcTmp = '\0';
		//		break;
		//	}
		//}
		//sProcName = szBuf;
		
		WSADATA wsaData;
		int iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
		if (iResult != NO_ERROR)
		{
			CRLog(E_ERROR,"Error at WSAStartup()");
			return -1;
		}

		
		DWORD dwProcesses[1024], dwNeeded, dwProcNum;          
		if ( !EnumProcesses( dwProcesses, sizeof(dwProcesses), &dwNeeded ) )
			return -1;

		char szTmp[2048];
		int nCount = 0;
		dwProcNum = dwNeeded / sizeof(DWORD);
	    for ( DWORD i = 0; i < dwProcNum; i++ )
		{
			memset(szTmp,0x00,sizeof(szTmp));
			GetProcessName(dwProcesses[i],szTmp,sizeof(szTmp));
			if(strcmp(szFileName,szTmp) == 0)
				nCount++;
		}
	
		//进程是否已存在
		if (nCount > 1)
		{
			CRLog(E_ERROR ,"已经启动了一个实例");
			return -1;
		}

		if(argc > 1 && 
			(strcasecmp(argv[1], "-d") == 0 ||
			strcasecmp(argv[1], "-i") == 0 ||
			strcasecmp(argv[1], "-u") == 0))
		{ // NT Service模式
			DWORD dwVersion = GetVersion();

			DWORD dwWindowsMajorVersion =  (DWORD)(LOBYTE(LOWORD(dwVersion)));
			DWORD dwWindowsMinorVersion =  (DWORD)(HIBYTE(LOWORD(dwVersion)));
			
			if (dwVersion < 0x80000000)
			{// Windows NT/2000/XP/2003
				CNTService GWService("NETWORK_MONITOR_PROCESS", "NmMon32/Process");
				if(!GWService.ParseStandardArgs(argc, argv))
				{
					GWService.StartService();
				}
				return 0;
			}
			else                                     
			{   // 不支持的Windows版本
				return 1;
			}
		}

		if (SetConsoleCtrlHandler((PHANDLER_ROUTINE)ConsoleHandler,TRUE)==FALSE)
		{
			cout << "Unable to install console msg handler!" << endl;
		}
#else
		signal(SIGPIPE, SIG_IGN);
		signal(SIGHUP, SIG_IGN);
		signal(SIGINT, SIG_IGN);
		//	signal(SIGTERM, sig_term);
		signal(SIGUSR1, SIG_IGN);
		signal(SIGUSR2, SIG_IGN);
		signal(SIGALRM, SIG_IGN);

		signal(SIGCHLD,SIG_IGN);
#endif

		if (0 == theMgr.Init())
		{
			theMgr.Start();
			theMgr.Run();
			theMgr.Stop();
			theMgr.Finish();

			
			//退出时未作所有线程同步，简单sleep
			msleep(1);
		}
#ifdef WIN32
		WSACleanup();
#endif
		return 0;
	}
	catch(std::exception e)
	{
		CRLog(E_CRITICAL,"exception:%s", e.what());
		return -1;
	}
	catch(...)
	{
		CRLog(E_CRITICAL,"Unknown exception");
		return -1;
	}
}