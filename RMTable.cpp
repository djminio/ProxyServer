#include "inetwork.h"
#include "RMTable.h"


CRMTable* g_pRMTable;
extern I4DyuchiNET* g_pINet;

extern void __stdcall OnConnectListenerSuccess( DWORD dwConnectionIndex, void *pData );
extern void __stdcall OnConnectListenerFail( void *pData );


CRMTable::CRMTable(DWORD num)
{
	m_dwMaxBucketNum = num;

	m_ppInfoTable = NULL;

	m_ppInfoTable = new RMCLIENT_INFO*[num];	//m_ppInfoTable은 (유저인포 내용의 주소값)의 주소값을 가르킨당.. 
	memset(m_ppInfoTable,0,sizeof(RMCLIENT_INFO*)*num);

	m_ListenerTable.Create(2);		//ConnectTable 생성하고..   2개..   하나는 접속 안한 Listener, 하나는 접속한 Listener

	m_bClientConnect = 0;
	m_CertainIPNum = 0;
	memset(m_RMCertIP, 0, sizeof(m_RMCertIP));
}

CRMTable::~CRMTable()
{
	RemoveAllClientTable();

	if (m_ppInfoTable)
	{
		delete [] m_ppInfoTable;
		m_ppInfoTable = NULL;
	}
}

bool CRMTable::AddClient(DWORD dwConnectionIndex, PACKET_RM_LOGIN* packet)
{
	if (!dwConnectionIndex)
		return FALSE;

	RMCLIENT_INFO* info = new RMCLIENT_INFO;	
	memset(info, 0, sizeof(RMCLIENT_INFO));		

	//접속정보 입력 
	info->bConnectType = RM_TYPE_TOOL;
	info->ConnectionIndex = dwConnectionIndex;
	//Modified by KBS 020330
	info->IPAddress = g_pINet->GetUserAddress(dwConnectionIndex)->sin_addr.s_addr;	//sin_addr.S_un.S_addr;	
	//
	info->dwID = info->IPAddress;
	strcpy(info->szLoginID, packet->ID);
	strcpy(info->szName, packet->Name);

		
	g_pINet->SetServerInfo(dwConnectionIndex,(void*)info);	//DLL에 Listener 정보 주소값 setting 시킴...

	AddClientInfo(info);		//HashTable에 추가
	return true;
}

BYTE CRMTable::GetClientNum()	//접속한 클라이언트 숫자를 리턴 
{
	return m_bClientConnect;
}

void CRMTable::AddClientInfo(RMCLIENT_INFO* info)
{													
	DWORD index = info->dwID%m_dwMaxBucketNum;		
	

	m_bClientConnect++;

	if (!m_ppInfoTable[index])	//m_ppInfoTable[index] 에 내용이 없으면 괄호 안으로.
	{
		m_ppInfoTable[index] = info;
		info->pNextUserInfo = NULL;
		info->pPrvUserInfo = NULL;
		return;
	}


	RMCLIENT_INFO* cur = m_ppInfoTable[index];
	RMCLIENT_INFO* prv = NULL;
	while (cur)
	{
		prv = cur;
		cur = cur->pNextUserInfo;
	}
	
	cur = prv->pNextUserInfo = info;
	cur->pPrvUserInfo = prv;
	cur->pNextUserInfo = NULL;
}

RMCLIENT_INFO* CRMTable::GetClientInfo(DWORD dwID)
{
	DWORD index = dwID%m_dwMaxBucketNum;

	RMCLIENT_INFO* cur = m_ppInfoTable[index];

	while (cur)
	{
		if (cur->dwID == dwID)
		{
			return cur;
		}
		cur = cur->pNextUserInfo;
	}
	return NULL;
}

void CRMTable::RemoveClient(DWORD dwConnectionIndex)
{
	//Modified by KBS 020330
	RMCLIENT_INFO* pInfo = (RMCLIENT_INFO*)g_pINet->GetUserInfo(dwConnectionIndex);
	RemoveClientID(pInfo->dwID);
}


void CRMTable::RemoveClientID(DWORD dwID)
{
	DWORD index = dwID%m_dwMaxBucketNum;

	RMCLIENT_INFO* cur  = m_ppInfoTable[index];
	RMCLIENT_INFO* next = NULL;
	RMCLIENT_INFO* prv  = NULL;
	
	while (cur)
	{
		prv = cur->pPrvUserInfo;
		next = cur->pNextUserInfo;
		if (cur->dwID == dwID)
		{
			if (!prv)	// if head
				m_ppInfoTable[index] = next;	
			else 
				prv->pNextUserInfo = next;
			
			if (next)
				next->pPrvUserInfo = prv;

			m_bClientConnect--;

			if(cur)	
				delete cur;		

			cur = NULL;
			return;

		}
		cur = cur->pNextUserInfo;
	}
	return;
}


void CRMTable::RemoveAllClientTable()
{
	RMCLIENT_INFO* cur = NULL;
	RMCLIENT_INFO* next = NULL;
	for (DWORD i=0; i<m_dwMaxBucketNum; i++)
	{
		cur = m_ppInfoTable[i];
		while (cur)
		{
			next = cur->pNextUserInfo;
			delete cur;
			cur = next;
		}
		m_ppInfoTable[i] = NULL;
	}

	m_bClientConnect = 0;	//Client 접속수 
}


void CRMTable::BroadcastAllRMClient(char* pMsg, DWORD dwLength)
{
	
	RMCLIENT_INFO* cur = NULL;
	RMCLIENT_INFO* next = NULL;
	for (DWORD i=0; i<m_dwMaxBucketNum; i++)
	{
		cur = m_ppInfoTable[i];
		while (cur)
		{
			next = cur->pNextUserInfo;
			g_pINet->SendToUser(cur->ConnectionIndex, pMsg, dwLength, FLAG_SEND_NOT_ENCRYPTION);
			cur = next;
		}
	}
}


//Disconnect된 모든 Listener에 접속을 시도한다.
void CRMTable::ConnectAllDisconnectedListener()
{
	RM_LISTENER_INFO* cur = NULL;
	RM_LISTENER_INFO* next = NULL;
	
	cur = m_ListenerTable.m_ppInfoTable[0];		//0이 접속 안한 상태이니...
	while (cur)
	{
		next = cur->pNextInfo;
		
		if(!m_ListenerTable.CheckDuplicateIP(cur->szIP, 0))	//중복되는거것이 아닐때만.. 
			m_ListenerTable.Add( cur->szIP , 0);	//접속 안한상태는 두번째 인자값 0으로 Add
		
		g_pINet->ConnectToServerWithServerSide( cur->szIP , PROXY_SERVER_CONNECTION_PORT, OnConnectListenerSuccess, OnConnectListenerFail, cur->szIP ); 
		//}
		cur = next;
	}
}

BOOL CRMTable::BroadcastAllListener(char *packet, DWORD dwLength)
{
	RM_LISTENER_INFO* cur = NULL;
	RM_LISTENER_INFO* next = NULL;
	
	cur = m_ListenerTable.m_ppInfoTable[ 1 ];		//1이 접속 한 상태이니...
	while (cur)
	{
		next = cur->pNextInfo;
						
		g_pINet->SendToServer(cur->dwConnectionIndex, packet, dwLength, FLAG_SEND_NOT_ENCRYPTION);
			
		cur = next;
	}

	return TRUE;
}

/*
//매개변수에 지정한 종류의 서버Listener로만 Broadcast
BOOL CRMTable::BroadcastEachListener(DWORD dwServerType, char *packet, DWORD dwLength)
{
	LP_SERVER_DATA pServerData;
	for( pServerData = this->m_pServerListHead; pServerData; pServerData = pServerData->pNextServerData )
	{
		if( pServerData->dwServerType == dwServerType )
		{
			g_pINet->SendToServer( pServerData->dwConnectionIndex , packet, dwLength, FLAG_SEND_NOT_ENCRYPTION);
		}
	}
	return TRUE;
}
*/

//ini파일로 부터 RMTool Login가능한 IP를 받아온다..
BOOL CRMTable::GetCertainIPFromIni()
{
	char path[ MAX_PATH ];
	
	sprintf(path,".\\ProxyServer.ini");                          
	m_CertainIPNum = (DWORD)GetPrivateProfileInt( "RMTool_Info", "CertainIPNum ", 0, path );
	if( m_CertainIPNum <= 0 ) 
		return FALSE;
		
	for(DWORD i = 0 ; i < m_CertainIPNum ; i ++ )
	{	
		char	keyname[ 32 ];
		sprintf( keyname, "CertainIP%d", i+1 );
		::GetPrivateProfileString( "RMTool_Info", keyname, "", m_RMCertIP[ i ], sizeof(m_RMCertIP[ i ]), path );
	}	

	return TRUE;
}

//접속 가능한 IP인지 체크..
BOOL CRMTable::CheckCertainIP(DWORD dwConnectionIndex, char* ip)
{
	// YGI		// 이중 비교
	return true;
	//Connection에서 얻은 IP
	char szConnectionIP[16];	memset(szConnectionIP , 0, 16);
	WORD wPort;
	g_pINet->GetUserAddress(dwConnectionIndex, szConnectionIP, &wPort);
	
	if(strcmp(szConnectionIP, ip))
		return FALSE;
	
	for(int i=0 ; i<m_CertainIPNum ; i++)
	{
		if(!strcmp(m_RMCertIP[i],ip))
			return TRUE;
	}
	
	return FALSE;
}

//해당 IP가 Listener table에 존재 하는가를 체크한다...    중복 안되게..
BOOL CRMTable::CheckExistIP(char *szIp)
{
	RM_LISTENER_INFO* cur = NULL;
	RM_LISTENER_INFO* next = NULL;
	
	for(int i=0; i<2 ; i++)
	{
		cur = m_ListenerTable.m_ppInfoTable[ i ];		
		while (cur)
		{
			next = cur->pNextInfo;
							
			if(!strcmp(cur->szIP, szIp))	return TRUE;
						
			cur = next;
		}
	}

	return FALSE;
}


//게임 서버 포트를 이용해 Listener의 ConnectionIndex를 얻어온다. 
DWORD CRMTable::GetListenerConnectionIndex( WORD wGameServerPort )
{
	//게임 서버의 Data를 찾아냄..
	LP_SERVER_DATA pServerData;
	for( pServerData = g_pServerTable->m_pServerListHead; pServerData; pServerData = pServerData->pNextServerData )
	{
		if( pServerData->wPort == wGameServerPort )
		{
			break;
		}
	}

	if(!pServerData)	return 0;

	RM_LISTENER_INFO* cur = NULL;
	RM_LISTENER_INFO* next = NULL;
	cur = m_ListenerTable.m_ppInfoTable[ 1 ];		//1이 접속 한 상태이니...
	while (cur)
	{
		next = cur->pNextInfo;
		if( !strcmp(cur->szIP, pServerData->szIP) )	//게임서버 IP와 ListenerIP가 같으면 
		{
			return cur->dwConnectionIndex;			//Listener의 dwConnectionIndex를 리턴 
		}
		cur = next;
	}
	return 0;
}