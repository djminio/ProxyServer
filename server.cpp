#include "server.h"
#include "typedef.h"
#include "usertable.h"
#include "servertable.h"
#include "network_guid.h"
#include "recvmsg.h"
#include "monitor.h"
#include "Proxy.h"
#include "dr_agent_structures.h"
#include "RMTable.h"
#include "RMDefine.h"
#include "UserManager.h"

I4DyuchiNET* g_pINet = NULL;
char buffer1[512];

WORD g_wCurDay;			//현재 날짜를 저장 하는 전역변수 
WORD g_wCurMonth;		//현재 달(month)을  저장 하는 전역변수 

extern CUserManager g_mgrUser;

void DisplayProxyData()
{
	MyLog( LOG_NORMAL, "TOTAL <%d/(Failed:%d)> CONNECTIONS FROM SERVER START", g_pProxy->dwTotalLogUser, g_pProxy->dwFailtoAllocUserNum );
	MyLog( LOG_NORMAL, "TOTAL <Users:%d/ServerMonitor:%d> ARE CONNECTIONS ARE AVAILABLE NOW", g_pUserTable->GetUserNum(), 0 );
	
	if(g_pProxy->bLimitLoginTryPerSec)
		MyLog( LOG_NORMAL, "Login Try Limit: %d users can login per sec now. F7 +, F8 -",g_pProxy->bLimitLoginTryPerSec);
	else MyLog( LOG_NORMAL, "Proxy Login unlimit");
	
	if (g_pProxy->dwFailtoAllocUserNum)
	{
		MyLog( LOG_FATAL, "-- TOTAL <%d> ALLOC FAILED CONNECTIONS", g_pProxy->dwFailtoAllocUserNum );
	}
	return;
}
//

void LogUserNumInfo()
{
	FILE *fp;
	static int max_total_user;
	DWORD total_user = g_pServerTable->GetNumOfUsersInServerSet();

	time_t lTime;
	struct tm *today;
	time( &lTime );
	today = localtime( &lTime );
	char tempname[ FILENAME_MAX];
	sprintf( tempname, "CurrentUserNumber_%04d%02d%02d.txt", today->tm_year+1900, today->tm_mon+1, today->tm_mday );
	fp = fopen( tempname, "at+");
	if( fp )
	{
#ifdef BUFF_BUG_CHECKS // Finito 26/08/07 buff bug checks to take away 12 hours when time changes
	if (today->tm_hour - g_hour >= 12 && g_hour != -1)
	{
		fprintf( fp, "%02d.%02d %02d:%02d %5d ",	today->tm_mon+1,	
													today->tm_mday,		
													today->tm_hour - 12,		
													today->tm_min,		
													total_user );	
	}
	else
	{
		fprintf( fp, "%02d.%02d %02d:%02d %5d ",	today->tm_mon+1,	
													today->tm_mday,		
													today->tm_hour,		
													today->tm_min,		
													total_user );	
	}
#else
		fprintf( fp, "%02d.%02d %02d:%02d %5d ",	today->tm_mon+1,	
													today->tm_mday,		
													today->tm_hour,		
													today->tm_min,		
													total_user );	
#endif	

		for( int i = 0 ; i < total_user / 50 ; i ++)
		{
			fprintf( fp, "*", fp);
		}
		fprintf( fp, "\n" );
		fclose(fp);
	}
}
void __stdcall ShowServerStatus(DWORD dwValue)//020511 lsw
{
	if( g_pServerTable )
	{
		LocalMgr.DisplayLocalizingSet();//021007 lsw
		g_pServerTable->ShowServerStatus();
		DisplayProxyData();
	}
	else
	{
		MyLog( LOG_JUST_DISPLAY, "<< g_pServerTable Is not available >>" );
	}
}

void __stdcall ShowServerStatusDetail(DWORD dwValue)//020511 lsw
{
	DWORD dwCount = 0;

	if( g_pServerTable )
	{
		LP_SERVER_DATA pDummyServerData = g_pServerTable->GetServerListHead();
		while( pDummyServerData )
		{
			dwCount++;
			MyLog( LOG_NORMAL, "<< %2d / %12s / %4d >> UsingDB: %4d(%6d), NumOfUsers: %4d, Status: %c >>"
				, dwCount
				, GetTypedServerText(pDummyServerData->dwServerType)
				, pDummyServerData->wPort
				, (pDummyServerData->pUsingDBDemon?pDummyServerData->pUsingDBDemon->wPort:0)
				, (pDummyServerData->pUsingDBDemon?g_pServerTable->GetDBLoad( pDummyServerData->pUsingDBDemon ):0)
				, pDummyServerData->dwNumOfUsers
				, ServerStatusSymbols[pDummyServerData->dwStatus] );

			pDummyServerData = pDummyServerData->pNextServerData;
		}
		MyLog( LOG_NORMAL, "-- Total %d Servers are listed", dwCount );
	}
	else
	{
		MyLog( LOG_JUST_DISPLAY, "<< g_pServerTable Is not available >>" );
	}
}

// DB Demon 로드를 주기적으로 조정한다
void __stdcall ReBalanceDBLoad(DWORD dwValue)//020511 lsw
{
	if( g_pServerTable )
	{
		g_pServerTable->ReBalanceDBDemonSettings();
	}
	else
	{
		MyLog( LOG_FATAL, "ReBalanceDBLoad() :: g_pServerTable Is NULL!!!" );
	}
	return;
}

// 모든 서버에게 상태 보고를 요구한다.
void __stdcall OrderToReportServerStatus(DWORD dwValue)//020511 lsw
{
	// add by slowboat
	LogUserNumInfo();

	LP_SERVER_DATA pCur = NULL;
	DWORD dwCount = 0;
	DWORD dwTotal = 0;

	pCur = g_pServerTable->GetServerListHead();
	
	// Thralas - Added code for webbased status
	char szStatusFile[256];
	FILE *pStatusFile;

	GetPrivateProfileString("extra", "statuspath", "" , szStatusFile, sizeof(szStatusFile), PROXY_SERVER_INI_);
	pStatusFile = fopen(szStatusFile, "w");

	if(pStatusFile != NULL)
	{
		fprintf(pStatusFile, "%d\n", g_pServerTable->GetNumOfUsersInServerSet());
	}

	while( pCur )
	{
		// Target은 접속된 모든 서버.
		if( pCur->dwConnectionIndex )
		{
			dwTotal++;
			if( g_pServerTable->OrderToReportServerStatus( pCur ) )
			{
				dwCount++;
			}
			else
			{
#ifdef __ON_DEBUG
//				_asm int 3;
#endif
			}
		}
		
		//Webbased status : Thralas
		if(pStatusFile != NULL)
		{
			fprintf(pStatusFile, "%d:%d\n", pCur->wPort, pCur->dwStatus);
		}

		pCur = pCur->pNextServerData;
		//////Serverstatus close fp

	}
	/*fclose(pStatusFile);*/ //causing problems with keeping the proxy server running error when closing statusfile does not contiunie to next line of code.
	MyLog( LOG_NORMAL, "PROXY Ordered to Servers to Report Server Status. (%d/%d) Servers are Listen.", dwCount, dwTotal);
	// added by slowboat
	// to calculate user num of server set. don't delete it.
	g_pServerTable->ShowServerStatus();

	return;
}


void __stdcall DestroyAllServers(DWORD dwValue)//020511 lsw
{
	LP_SERVER_DATA pDummyServerData;

	if( !g_pServerTable )
	{
		return;
	}

	for( pDummyServerData = g_pServerTable->GetServerListHead(); pDummyServerData; pDummyServerData = pDummyServerData->pNextServerData )
	{
		if( pDummyServerData->dwConnectionIndex )
		{
			g_pServerTable->DestroyOtherServer( pDummyServerData );
		}
	}
	return;
}

void __stdcall ShowServerConnections(DWORD dwValue)//020511 lsw
{
	if( !g_pServerTable )
	{
		return;
	}
	else
	{
		g_pServerTable->ShowServerConnectionStatus();
	}
}

void __stdcall TimerForUserTable(DWORD dwValue)//020511 lsw
{
	g_mgrUser.DelLogout(); // CSD-030509
	g_pProxy->bTryLoginThisSec = 0; // Reset

	if( g_pUserTable )
	{
		// 접속종료 대기 처리용.
		DWORD dwCount = g_pUserTable->CloseConnectionWithAwaitingToDisconnect();
		if( dwCount )
		{
			// 맞는다고 생각하고 로그하지 않는다.
//			MyLog( LOG_NORMAL, "TimerForUserTable() :: (%d/%d) Timed out connections are cleared.", dwCount, g_pUserTable->GetUserNum() );
		}
	}
	return;
}
void __stdcall IncreseLimitUserLogin(DWORD dwValue)//020511 lsw
{
	if (g_pProxy->bLimitLoginTryPerSec < 30)
		g_pProxy->bLimitLoginTryPerSec++;

	if (g_pProxy->bLimitLoginTryPerSec)
		MyLog( LOG_NORMAL, "Login Try LIMIT: %d users can login per sec now. F7 +, F8 -",g_pProxy->bLimitLoginTryPerSec);
	else 
		MyLog( LOG_NORMAL, "Login Try UNLIMIT. F7 +, F8 -");
}
void __stdcall DecreseLimitUserLogin(DWORD dwValue)//020511 lsw
{
	if (g_pProxy->bLimitLoginTryPerSec)
		g_pProxy->bLimitLoginTryPerSec--;

	if (g_pProxy->bLimitLoginTryPerSec)
		MyLog( LOG_NORMAL, "Login Try LIMIT: %d users can login per sec now. F7 +, F8 -",g_pProxy->bLimitLoginTryPerSec);
	else MyLog( LOG_NORMAL, "Login Try UNLIMIT. F7 +, F8 -");
}

void __stdcall IncreseMaxUser(DWORD dwValue)//020511 lsw
{
	g_pProxy->dwMaxUser+=25;
	MyLog( LOG_NORMAL, "MAX USER ADJUST: %d users can login this set. F3 -, F4 +",g_pProxy->dwMaxUser);
}
void __stdcall DecreseMaxUser(DWORD dwValue)//020511 lsw
{
	if (g_pProxy->dwMaxUser)
		g_pProxy->dwMaxUser-=25;
	MyLog( LOG_NORMAL, "MAX USER ADJUST: %d users can login this set. F3 -, F4 +",g_pProxy->dwMaxUser);
}

// 011012 KBS ; 날짜 변경 Check 함수 
void __stdcall CheckDay(DWORD dwValue)//020511 lsw
{
	SYSTEMTIME time;
	GetLocalTime(&time);
	BYTE	bChanged = 1;		//날(day)만 변경되었으면 1, 날과 달(month) 둘다 변경 되었으면 2

	//달이 바뀌었을때	
	if(time.wMonth != g_wCurMonth)
	{
		g_wCurMonth = time.wMonth;		//달(month) 갱신 
		bChanged++;
	}
	
	//날짜가 바뀌었을때 
	if(time.wDay != g_wCurDay)
	{
		g_wCurDay = time.wDay;			//날짜 갱신 
		
		t_send_map_change_date packet(bChanged, g_wCurDay, g_wCurMonth);
		//g_pServerTable->BroadCastToEveryServer((char*)&packet, packet.GetPacketSize(), SERVER_TYPE_MAP);
		g_pServerTable->BroadCastMapServer((char*)&packet, packet.GetPacketSize());

		//g_pINet->SendToServer(1, (char*)&packet, packet.GetPacketSize(), FLAG_SEND_NOT_ENCRYPTION);
	}
}
//

// 011012 KBS ; 서버 Start시에 현재 달과 날짜를 셋팅하는 함수 
void SetCurrentDate()
{
	SYSTEMTIME time;
	GetLocalTime(&time);

	g_wCurMonth = time.wMonth;
	g_wCurDay = time.wDay;

}
//

//Added by KBS 011213

void StartEchoTimer()	{	g_pINet->ResumeTimer(4);	}
void StopEchoTimer()	{	g_pINet->PauseTimer(4);		}
void StartWaitTimer()	{	g_pINet->ResumeTimer(5);	}
void StopWaitTimer()	{	g_pINet->PauseTimer(5);		}

EchoBucket g_EchoBucket[ MAX_SERVER_NUM ];	//Echo 메세지 받을 Bucket
DWORD g_dwEchoID = 0;

void __stdcall SendEcho(DWORD dwValue)//020511 lsw
{
	g_dwEchoID++;									//Echo ID 증가 
	memset(g_EchoBucket, 0, sizeof(EchoBucket));	//Bucket 초기화 	
	
	LP_SERVER_DATA pTargetServerData;
	DWORD dwFailCounter = 0;	int count=0;
	
	for( pTargetServerData = g_pServerTable->m_pServerListHead; pTargetServerData;  pTargetServerData = pTargetServerData->pNextServerData )
	{
		if( (pTargetServerData->dwConnectionIndex) && (pTargetServerData->dwStatus == STATUS_ACTIVATED) )
		{
			g_EchoBucket[ count ].bSended = 1;
			g_EchoBucket[ count ].wPort = pTargetServerData->wPort;
			
			PACKET_ECHO packet(count, pTargetServerData->wPort, g_dwEchoID);
			if( !g_pINet->SendToServer( pTargetServerData->dwConnectionIndex, 
				(char*)&packet, packet.GetPacketSize(), FLAG_SEND_NOT_ENCRYPTION ) )
			{
				dwFailCounter++;
			}

			count++;	//Bucket Index 증가 
		}
	}
	
	StartWaitTimer();
}

void __stdcall CheckEchoMessage(DWORD dwValue)//020511 lsw
{
	StopWaitTimer();		

	DWORD dwShit = 0;
	for(int i=0; i<MAX_SERVER_NUM ; i++)
	{
		if(g_EchoBucket[ i ].bSended && !g_EchoBucket[ i ].bReceived)	//응답 메세지 못받은 서버..  Down 간주!!
		{
			//여기서 RMClient에 알림 작업 
			PACKET_DOWN_SERVER packet( g_pServerTable->m_dwServerSetNumber, g_EchoBucket[ i ].wPort);
			g_pRMTable->BroadcastAllRMClient((char*)&packet, packet.GetPacketSize());	
			
			MyLog( LOG_NORMAL, "								   ");
			MyLog( LOG_NORMAL, "Emergency!!!!! Down Server!!!! : %d", g_EchoBucket[ i ].wPort);
			MyLog( LOG_NORMAL, "								   ");
		}
	}
}


//

bool StartProxyServer()
{
	// 011012 KBS ; 현재 Day 셋팅
	SetCurrentDate();
	//

	g_pProxy = new CProxy;

	//Modified by KBS 011213
	CUSTOM_EVENT ev[14];
	ev[0].dwPeriodicTime = CONNECTION_CHECK_INTERVAL;
	ev[0].pEventFunc = OrderToReportServerStatus;

	ev[1].dwPeriodicTime = 300000;	// 5분
	ev[1].pEventFunc = ReBalanceDBLoad;

	ev[2].dwPeriodicTime = 1000;	// 1분
	ev[2].pEventFunc = TimerForUserTable;

	// 011012 KBS ; 날짜 변경 체크 Event 5분 
	ev[3].dwPeriodicTime = 300000;	
	ev[3].pEventFunc = CheckDay;
	//

	//011213 KBS
	ev[4].dwPeriodicTime = 60000;	//1분마다 
	ev[4].pEventFunc = SendEcho;

	ev[5].dwPeriodicTime = 15000;	//15초 마다		Modified by KBS 020330
	ev[5].pEventFunc = CheckEchoMessage;
	//KBS

	ev[6].dwPeriodicTime = 0;
	ev[6].pEventFunc = ShowServerStatus;

	ev[7].dwPeriodicTime = 0;
	ev[7].pEventFunc = DestroyAllServers;

	ev[8].dwPeriodicTime = 0;
	ev[8].pEventFunc = ShowServerStatusDetail;

	ev[9].dwPeriodicTime = 0;
	ev[9].pEventFunc = ShowServerConnections;
	
	ev[10].dwPeriodicTime = 0;	// By Key
	ev[10].pEventFunc = IncreseLimitUserLogin;
	
	ev[11].dwPeriodicTime = 0;	// By Key
	ev[11].pEventFunc = DecreseLimitUserLogin;
	
	ev[12].dwPeriodicTime = 0;	// By Key
	ev[12].pEventFunc = IncreseMaxUser;
	
	ev[13].dwPeriodicTime = 0;	// By Key
	ev[13].pEventFunc = DecreseMaxUser;

	/*
	CUSTOM_EVENT ev[12];

	ev[0].dwPeriodicTime = 0;
	ev[0].pEventFunc = ShowServerStatus;

	ev[1].dwPeriodicTime = 0;
	ev[1].pEventFunc = DestroyAllServers;

	ev[2].dwPeriodicTime = 0;
	ev[2].pEventFunc = ShowServerStatusDetail;

	ev[3].dwPeriodicTime = 0;
	ev[3].pEventFunc = ShowServerConnections;

	ev[4].dwPeriodicTime = CONNECTION_CHECK_INTERVAL;
	ev[4].pEventFunc = OrderToReportServerStatus;

	ev[5].dwPeriodicTime = 300000;	// 5분
	ev[5].pEventFunc = ReBalanceDBLoad;

	ev[6].dwPeriodicTime = 1000;	// 1분
	ev[6].pEventFunc = TimerForUserTable;
	
	ev[7].dwPeriodicTime = 0;	// By Key
	ev[7].pEventFunc = IncreseLimitUserLogin;
	ev[8].dwPeriodicTime = 0;	// By Key
	ev[8].pEventFunc = DecreseLimitUserLogin;
	ev[9].dwPeriodicTime = 0;	// By Key
	ev[9].pEventFunc = IncreseMaxUser;
	ev[10].dwPeriodicTime = 0;	// By Key
	ev[10].pEventFunc = DecreseMaxUser;

	// 011012 KBS ; 날짜 변경 체크 Event 5분 
	ev[11].dwPeriodicTime = 300000;	
	ev[11].pEventFunc = CheckDay;
	//
	*/
	
	DESC_NETWORK desc;
	desc.OnAcceptServer = OnAcceptServer;
	desc.OnAcceptUser = OnAcceptUser;
	desc.OnDisconnectServer = OnDisconnectServer;
	desc.OnDisconnectUser = OnDisconnectUser;
	desc.dwMainMsgQueMaxBufferSize = 5120000;
	desc.dwMaxServerNum = 120;			//Modified at 020111
	desc.dwMaxUserNum = 4000;
	desc.dwServerBufferSizePerConnection = 256000;
	desc.dwServerMaxTransferSize = 65000;
	desc.dwUserBufferSizePerConnection = 65000;
	desc.dwUserMaxTransferSize = 8192;
	desc.OnRecvFromServerTCP = ReceivedMsgServer;
	desc.OnRecvFromUserTCP = ReceivedMsgUser;
	desc.dwCustomDefineEventNum = 14;	// 011012 KBS	; Event 갯수 14개로 변경 
	desc.pEvent = ev;
	desc.dwConnectNumAtSameTime = 200;	//Modified at 020111
//	desc.dwFlag = NETDDSC_DEBUG_LOG;
//	NETDDSC_DEBUG_LOG

	HRESULT hr;
	CoInitialize(NULL);
    
	hr = CoCreateInstance(
           CLSID_4DyuchiNET,
           NULL,
           CLSCTX_INPROC_SERVER,
           IID_4DyuchiNET,
           (void**)&g_pINet);


	if (FAILED(hr))
	{
		MyLog( LOG_FATAL, "FAILED : NO DLL IN THAT SYSTEM");
		return false;
	}

	// 유져테이블 생성 부분.무시하자.
	g_pUserTable = new CUserTable(MAX_USER_NUM);
	g_pServerTable = new CServerTable(PROXY_SERVER_INI_,MAX_SERVER_NUM,g_pINet);//021007 lsw
	
	// 011106 KBS
	g_pRMTable = new CRMTable( MAX_LISTENER_NUM );
	g_pRMTable->GetCertainIPFromIni();
	


	if( !g_pUserTable ) return false;
	if( !g_pServerTable || !g_pServerTable->IsServerRunning() ) return false;

	// added by slowboat 2001.2.28
	g_pProxy->bLimitLoginTryPerSec = g_pServerTable->GetMaxUserNumPerSec();
	g_pProxy->dwMaxUser = g_pServerTable->GetMaxUserNum();

	MyLog( LOG_NORMAL, "CServerTable Initialized..." );

	if (!g_pINet->CreateNetwork(&desc,10,10))
		return false;

	MyLog( LOG_NORMAL, "INetwork Initialized..." );
	
	//Added KBS 011213
	//StopWaitTimer(); // 040406 kyo
	//StopEchoTimer(); // 040406 kyo
	//


	/*
	g_pProxy->hKeyEvent[0] = g_pINet->GetCustomEventHandle(0);
	g_pProxy->hKeyEvent[1] = g_pINet->GetCustomEventHandle(1);
	g_pProxy->hKeyEvent[2] = g_pINet->GetCustomEventHandle(2);
	g_pProxy->hKeyEvent[3] = g_pINet->GetCustomEventHandle(3);
	g_pProxy->hKeyEvent[4] = g_pINet->GetCustomEventHandle(7);
	g_pProxy->hKeyEvent[5] = g_pINet->GetCustomEventHandle(8);
	g_pProxy->hKeyEvent[6] = g_pINet->GetCustomEventHandle(9);
	g_pProxy->hKeyEvent[7] = g_pINet->GetCustomEventHandle(10);
	*/

	g_pProxy->hKeyEvent[0] = g_pINet->GetCustomEventHandle(6);
	g_pProxy->hKeyEvent[1] = g_pINet->GetCustomEventHandle(7);
	g_pProxy->hKeyEvent[2] = g_pINet->GetCustomEventHandle(8);
	g_pProxy->hKeyEvent[3] = g_pINet->GetCustomEventHandle(9);
	g_pProxy->hKeyEvent[4] = g_pINet->GetCustomEventHandle(10);
	g_pProxy->hKeyEvent[5] = g_pINet->GetCustomEventHandle(11);
	g_pProxy->hKeyEvent[6] = g_pINet->GetCustomEventHandle(12);
	g_pProxy->hKeyEvent[7] = g_pINet->GetCustomEventHandle(13);

	// batch connect to other server
	MyLog( LOG_NORMAL, "Now Starting BatchConnect()...(It Takes some time)");
	DWORD dwWorkingServers = g_pServerTable->BatchConnect();

	g_pServerTable->SetServerStatus( STATUS_ACTIVATED );

	return true;
}

void EndProxyServer()
{
	if (g_pINet)
	{
		g_pINet->Release();
		g_pINet = NULL;
	}

	if (g_pUserTable)
	{
		delete g_pUserTable;
		g_pUserTable = NULL;
	}
	if (g_pServerTable)
	{
		delete g_pServerTable;
		g_pServerTable = NULL;
	}

	delete g_pProxy;
	g_pProxy = NULL;

	// 011109 KBS : Free Listener Table 
	delete g_pRMTable;
	g_pRMTable = NULL;
	//

	CoFreeUnusedLibraries();

	CoUninitialize();
}