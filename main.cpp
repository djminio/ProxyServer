#include "stdafx.h"
#include "server.h"
#include "monitor.h"
#include "proxy.h"
#include "servertable.h"
#include "MyLog.h"
#include "dr_agent_structures.h"

//Added by KBS 011119
#include "RMTable.h"

HANDLE hIn;


//Added by KBS 011206
void DestroyAllDBDemon()
{
	LP_SERVER_DATA pDummyServerData;

	if( !g_pServerTable )
	{
		return;
	}

	for( pDummyServerData = g_pServerTable->GetServerListHead(); pDummyServerData; pDummyServerData = pDummyServerData->pNextServerData )
	{
		if( pDummyServerData->dwConnectionIndex && pDummyServerData->dwServerType == SERVER_TYPE_DB )
		{
			g_pServerTable->DestroyDBDemon( pDummyServerData );
		}
	}
	return;
}
//
//Added by KBS 020118
void DestroyAgent()
{
	LP_SERVER_DATA pDummyServerData;

	if( !g_pServerTable )
	{
		return;
	}

	for( pDummyServerData = g_pServerTable->GetServerListHead(); pDummyServerData; pDummyServerData = pDummyServerData->pNextServerData )
	{
		if( pDummyServerData->dwConnectionIndex  && pDummyServerData->dwServerType == SERVER_TYPE_AGENT )
		{
			g_pServerTable->DestroyOtherServer( pDummyServerData );
//			break;//020610 lsw//020822 lsw
		}
	}
	return;
}
void DestroyAllMapserver()
{
	LP_SERVER_DATA pDummyServerData;

	if( !g_pServerTable )
	{
		return;
	}

	for( pDummyServerData = g_pServerTable->GetServerListHead(); pDummyServerData; pDummyServerData = pDummyServerData->pNextServerData )
	{
		if( pDummyServerData->dwConnectionIndex  && pDummyServerData->dwServerType == SERVER_TYPE_MAP )
		{
			g_pServerTable->DestroyOtherServer( pDummyServerData );
		}
	}
	return;
}
//
int main()
{
	INPUT_RECORD	irBuffer;
	memset(&irBuffer,0,sizeof(INPUT_RECORD));
	DWORD	dwResult;
	
	AllocConsole();
	
	hIn = GetStdHandle(STD_INPUT_HANDLE);
	g_hOut = GetStdHandle(STD_OUTPUT_HANDLE);

	SetMonitorSize( SCREEN_TEXT_SIZE_X, SCREEN_TEXT_SIZE_Y );

	InitMyLog();
	
	char szNationName[100] = {0,} ;//021007 lsw
	if( GetPrivateProfileString( "NATION_SET", "NATION_NAME", "" , szNationName, 50,PROXY_SERVER_INI_ ) )
	{
		bool bIsFree = (bool)GetPrivateProfileInt( "NATION_SET", "BETA_SERVER", 0, PROXY_SERVER_INI_ );
		if(!LocalMgr.InitVersion(szNationName,bIsFree)){return false;}
	}
	else
	{
		return false;
	}

	if( !StartProxyServer() )
	{
		goto FinishProxyServer;
	}

	g_pINet->ResumeTimer( 4 );
	g_pINet->ResumeTimer( 5 );
	g_pINet->ResumeTimer( 6 );

	while ( g_pServerTable->IsServerRunning() )
	{
		ReadConsoleInput(hIn,&irBuffer,1,&dwResult);
	
		if (irBuffer.EventType == KEY_EVENT)
		{
			if (irBuffer.Event.KeyEvent.bKeyDown)
			{
				switch ( irBuffer.Event.KeyEvent.wVirtualKeyCode )
				{
				case VK_ESCAPE:
					{
						if( MessageBox( NULL, "Are you sure to DESTROY this Proxy server?", "IMPORTANT", MB_YESNO ) == IDYES )
							g_pServerTable->DestroyServer( FINISH_TYPE_NORMAL );
					}break;
				case VK_F1:
					{
						SetEvent(g_pProxy->hKeyEvent[0]);
					}break;
				case VK_F2:
					{
						SetEvent(g_pProxy->hKeyEvent[2]);
					}break;
				case VK_F3:
					{
						// Decrese Max User
						SetEvent(g_pProxy->hKeyEvent[7]);
					}break;
				case VK_F4:
					{
						// Increse Max User
						SetEvent(g_pProxy->hKeyEvent[6]);
					}break;
					
				case VK_F5:
					{
						if ( g_pServerTable->ToggleUserAcceptAllowed() == true )
						{
							MyLog( LOG_NORMAL, "USER ACCEPT ALLOWED by Key Event" );
						}
						else 
						{
							MyLog( LOG_NORMAL, "USER ACCEPT STOPED by Key Event" );
						}
					}break;
				case VK_F6:
					{
						// Increse Limit
						SetEvent(g_pProxy->hKeyEvent[4]);	
					}break;
				case VK_F7:
					{
						// Decrese Limit
						SetEvent(g_pProxy->hKeyEvent[5]);
					}break;
				case VK_F8:
					{
						// Increse Limit
						//SetEvent(g_pProxy->hKeyEvent[4]);	
						//if( MessageBox( NULL, "WARNING!!! Are you sure Close One AgentServer?", "IMPORTANT", MB_YESNO ) == IDYES )
						if( MessageBox( NULL, "WARNING!!! Are you sure Close All AgentServer?", "IMPORTANT", MB_YESNO ) == IDYES )//020822 lsw
							DestroyAgent();
					}break;
				case VK_F9:
					{ 
						if( MessageBox( NULL, "WARNING!!! Are you sure to DESTROY All DBDemon?", "IMPORTANT", MB_YESNO ) == IDYES )
							DestroyAllDBDemon();
					}break;
				case VK_F10:
					{
						if( MessageBox( NULL, "WARNING!!! Are you sure to DESTROY ALL Map SERVERS?", "IMPORTANT", MB_YESNO ) == IDYES )
							DestroyAllMapserver();
							//SetEvent(g_pProxy->hKeyEvent[1]);
					}break;
				case VK_F11:
					{
						MyLog( LOG_JUST_DISPLAY, " " );
						MyLog( LOG_JUST_DISPLAY, "Attempt all disconnected Listener....." );
						MyLog( LOG_JUST_DISPLAY, " " );
						g_pRMTable->ConnectAllDisconnectedListener();
					}break;
				case VK_F12:
					{
						g_pServerTable->ShowListenerStatus();
					}break;
				default:break;
				}
			}
		}
	}

FinishProxyServer:
	
	MyLog( LOG_NORMAL, "Deleting ProxyServer Resources..." );
	EndProxyServer();
	MyLog( LOG_NORMAL, "*** PROXY SERVER Finished Successfully." );
	FreeMyLog();
	FreeConsole();
	return 0;
}

