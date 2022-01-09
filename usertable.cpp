// ----------------------------------
// CUserTable by chan78 at 2001/01/20
// ----------------------------------

#include "usertable.h"
#include "servertable.h"
#include "monitor.h"
#include "mylog.h"
#include <crtdbg.h>
#include "dr_agent_structures.h"

#ifdef __IS_PROXY_SERVER
#include "Server.h"
#endif

#ifdef __IS_AGENT_SERVER
#include "AgentServer.h"
#endif

CUserTable* g_pUserTable = NULL;

CUserTable::CUserTable(WORD wMaxBucketNum )
{
	m_wMaxBucketNum = wMaxBucketNum;
	m_dwDisconnectTick = 0;
	m_dwUserNum = 0;

	m_ppInfoTable = NULL;

	for( int i = 0; i<SIZE_OF_USER_DISCONNECT_TICK; i++ )
		this->m_pAwaitingDisconnectUserList[i] = NULL;

	m_dwUserNum = 0;

	m_ppInfoTable = new USERINFO*[m_wMaxBucketNum];
	memset(m_ppInfoTable,0,sizeof(USERINFO*)*m_wMaxBucketNum);
}

CUserTable::~CUserTable()
{
	RemoveAllUserTable();

	if (m_ppInfoTable)
	{
				
		delete [] m_ppInfoTable;
		m_ppInfoTable = NULL;
	}
}

DWORD CUserTable::IncreaseDisconnectTick()
{
	this->m_dwDisconnectTick++;

	if( this->m_dwDisconnectTick >= SIZE_OF_USER_DISCONNECT_TICK )
		this->m_dwDisconnectTick = 0;

	return this->m_dwDisconnectTick;
}

// for login server
DWORD CUserTable::AddUser(DWORD dwConnectionIndex)
{
	char txt[128];
	static DWORD dwNewID = 0;

	memset(txt,0,128);

	if (!dwConnectionIndex)
		return false;

#ifdef __ON_DEBUG
	// dwConnectionIndex 가 혹시 중복되지 않는지 Check
	if( g_pINet->GetUserInfo( dwConnectionIndex ) )
	{
//		_asm int 3;
	}
#endif

	dwNewID++;
	if( dwNewID == 0 )
	{
		dwNewID++;
	}

	USERINFO* info = new USERINFO;

	_ASSERT( info );

	// Initialize
	memset(info,0,sizeof(USERINFO));
	info->dwConnectionIndex = 0;
	info->dwID = dwNewID;
	info->dwStatus = STATUS_USER_ACTIVATED;

#ifdef __IS_AGENT_SERVER
	// memset 하므로 굳이 해줄 필요는 없지만...
	info->dwDBDemonConnectionIndex = 0;
	info->dwMapServerConnectionIndex = 0;
#endif

	info->dwConnectionIndex = dwConnectionIndex;
	info->wUDPPort = 0;
	info->dwAddress = g_pINet->GetUserAddress(dwConnectionIndex)->sin_addr.S_un.S_addr;
	info->bNameLength = 0;

	// g_pINet 에 셋팅.
	g_pINet->SetUserInfo(dwConnectionIndex,(void*)info);

	// 정보 얻기
	sockaddr_in* pSockAddrIn;

	pSockAddrIn = g_pINet->GetUserAddress(info->dwConnectionIndex);
	if( !(pSockAddrIn) )
	{
		delete info;
		return 0;
	}
	else
	{
		sprintf( info->szIP, "%d.%d.%d.%d"
			, pSockAddrIn->sin_addr.S_un.S_un_b.s_b1
			, pSockAddrIn->sin_addr.S_un.S_un_b.s_b2
			, pSockAddrIn->sin_addr.S_un.S_un_b.s_b3
			, pSockAddrIn->sin_addr.S_un.S_un_b.s_b4);
		info->wPort = pSockAddrIn->sin_port;
	}

	// Count User Num
	m_dwUserNum++;
	g_pServerTable->SetNumOfUsers( m_dwUserNum );

	AddUserInfo(info);

	return dwNewID;
}
void CUserTable::AddUserInfo( USERINFO* info )
{

	DWORD index = info->dwID%m_wMaxBucketNum;
	
	if (!m_ppInfoTable[index])
	{
		m_ppInfoTable[index] = info;
		info->pNextUserInfo = NULL;
		info->pPrvUserInfo = NULL;
		return;
	}

	USERINFO* cur = m_ppInfoTable[index];
	USERINFO* prv = NULL;

	while (cur)
	{
		prv = cur;
		cur = cur->pNextUserInfo;
	}
	
	cur = prv->pNextUserInfo = info;
	cur->pPrvUserInfo = prv;
	cur->pNextUserInfo = NULL;

	return;
}

USERINFO* CUserTable::GetUserInfo( DWORD dwID )
{
	DWORD index = dwID%m_wMaxBucketNum;

	USERINFO* cur = m_ppInfoTable[index];

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


void CUserTable::RemoveUser( DWORD dwConnectionIndex )
{
	USERINFO* pInfo = (USERINFO*)g_pINet->GetUserInfo(dwConnectionIndex);
#ifdef __ON_DEBUG
	_ASSERT(pInfo);
#else
	if( !pInfo )
	{
		// 이젠 절대로 생겨선 안되는 상황이다.
		MyLog( LOG_FATAL, "RemoveUser() :: pInfo is NULL!!! Notify to SERVER TEAM" );
		return;
	}
#endif
	RemoveUserID(pInfo->dwID);
}

void CUserTable::RemoveUserID( DWORD dwID )
{
	DWORD index = dwID%m_wMaxBucketNum;

	USERINFO* cur = m_ppInfoTable[index];
	USERINFO* next = NULL;
	USERINFO* prv = NULL;
	
	while (cur)
	{
		prv = cur->pPrvUserInfo;
		next = cur->pNextUserInfo;
		if (cur->dwID == dwID)
		{
			// Modified by chan78 at 2001/02/28
			// 접속종료 대기중인 경우의 처리.
			this->RemoveUserFromAwaitingDisconnectUserList( cur );

			// Hashtable 에서 제거.
			if (!prv)	// if head
				m_ppInfoTable[index] = next;
			else 
				prv->pNextUserInfo = next;
			
			if (next)
				next->pPrvUserInfo = prv;

			m_dwUserNum--;
			g_pServerTable->SetNumOfUsers( m_dwUserNum );

			MyLog( LOG_IGNORE, "User (dwID:%d, szID:%s, dwConnectionIndex:%d) Removed From CUserTable", cur->dwID, cur->szName, cur->dwConnectionIndex );

			// g_pINet 에 셋팅된 UserInfo 를 제거
			g_pINet->SetUserInfo( cur->dwConnectionIndex, NULL );

			delete cur;
			cur = NULL;

			return;
		}
		cur = cur->pNextUserInfo;
	}
	return;
}

void CUserTable::RemoveAllUserTable()
{
	USERINFO* cur = NULL;
	USERINFO* next = NULL;
	for (DWORD i=0; i<m_wMaxBucketNum; i++)
	{
		cur = m_ppInfoTable[i];
		while (cur)
		{
			if( cur->pAwaitingDisconnectUserInfoList )
				delete cur->pAwaitingDisconnectUserInfoList;

			next = cur->pNextUserInfo;
			delete cur;
			cur = next;
		}
		m_ppInfoTable[i] = NULL;
	}
}

void CUserTable::RemoveUserFromAwaitingDisconnectUserList( USERINFO *pUserInfo )
{
	if( pUserInfo->pAwaitingDisconnectUserInfoList )
	{
		USERINFO_LIST* pPrvUIList = pUserInfo->pAwaitingDisconnectUserInfoList->pPrvUserInfoList;
		USERINFO_LIST* pNextUIList = pUserInfo->pAwaitingDisconnectUserInfoList->pNextUserInfoList;

		if( !pPrvUIList && pNextUIList )
		{
			pNextUIList->pPrvUserInfoList = NULL;
			this->m_pAwaitingDisconnectUserList[pUserInfo->pAwaitingDisconnectUserInfoList->dwTick] = pNextUIList;
		}
		else if( pPrvUIList && !pNextUIList )
		{
			pPrvUIList->pNextUserInfoList = NULL;
		}
		else if( !pPrvUIList && !pNextUIList )
		{
			this->m_pAwaitingDisconnectUserList[pUserInfo->pAwaitingDisconnectUserInfoList->dwTick] = NULL;
		}
		else if( pPrvUIList && pNextUIList )
		{
			pPrvUIList->pNextUserInfoList = pNextUIList;
			pNextUIList->pPrvUserInfoList = pPrvUIList;
		}

		delete pUserInfo->pAwaitingDisconnectUserInfoList;
		pUserInfo->pAwaitingDisconnectUserInfoList = NULL;
	}
	return;
}



void CUserTable::DisconnectUserImmediately( DWORD dwConnectionIndex )
{
	g_pINet->CompulsiveDisconnectUser( dwConnectionIndex );
	return;
}


void CUserTable::DisconnectUserImmediately( USERINFO *pUserInfo )
{
	_ASSERT( pUserInfo );
	g_pINet->CompulsiveDisconnectUser( pUserInfo->dwConnectionIndex );
	return;
}

DWORD CUserTable::GetUserNum()
{
	return m_dwUserNum;
}

USERINFO_LIST* CUserTable::GetUserInfoList( DWORD dwTick )
{
	return this->m_pAwaitingDisconnectUserList[dwTick];
}

bool CUserTable::DisconnectUserBySuggest( USERINFO* pUserInfo )
{
	USERINFO_LIST *pUserInfoList;

	if( !pUserInfo )
	{
		return false;
	}

	if( pUserInfo->dwConnectionIndex == 0 )
	{
		// dwConnectionIndex가 0인 pUserInfo는 있을 수 없다. 
#ifdef __ON_DEBUG
//		_asm int 3;
#endif
		MyLog( LOG_FATAL, "DisconnectUserBySuggest() :: pUserInfo->dwConnectionIndex is NULL!!!!" );
		g_pUserTable->RemoveUserID( pUserInfo->dwID );
	}

	// Modified by chan78 at 2001/02/21
	pUserInfo->dwStatus = STATUS_USER_AWAITING_DISCONNECT;

	pUserInfoList = new USERINFO_LIST;
	memset( pUserInfoList, 0, sizeof( USERINFO_LIST ) );

	// pUserInfoList 값을 채운다.
	pUserInfoList->pUserInfo = pUserInfo;
	pUserInfoList->dwTick = this->m_dwDisconnectTick;

	// pUserInfo 와 연결한다.
	pUserInfo->pAwaitingDisconnectUserInfoList = pUserInfoList;

	if( this->m_pAwaitingDisconnectUserList[this->m_dwDisconnectTick] )
	{
		// 헤드에 새 USERINFO_LIST 를 넣는다.
		pUserInfoList->pNextUserInfoList = this->m_pAwaitingDisconnectUserList[this->m_dwDisconnectTick];
		this->m_pAwaitingDisconnectUserList[this->m_dwDisconnectTick]->pPrvUserInfoList = pUserInfoList;
	}

	this->m_pAwaitingDisconnectUserList[this->m_dwDisconnectTick] = pUserInfoList;

	return true;
}

bool CUserTable::DisconnectUserBySuggest( USERINFO* pUserInfo, WORD wRajaCmdNum )
{
	t_packet packet;

	// Build Packet
	packet.h.header.type = wRajaCmdNum;
	packet.h.header.size = 0;
	packet.h.header.crc = 0;

	if( !pUserInfo )
	{
		return false;
	}

	if( pUserInfo->dwConnectionIndex == 0 )
	{
		// dwConnectionIndex가 0인 pUserInfo는 있을 수 없다. 
		
		//_asm int 3;
		MyLog( LOG_FATAL, "DisconnectUserBySuggest() :: pUserInfo->dwConnectionIndex is NULL!!!!" );
		g_pUserTable->RemoveUserID( pUserInfo->dwID );
	}

	if( !g_pUserTable->SendToUser( pUserInfo, (char *)&packet, 5 ) )
	{
		MyLog( LOG_IMPORTANT, "DisconnectUserBySuggest() :: Failed To Suggest... Disconnect it." );
		g_pINet->CompulsiveDisconnectUser( pUserInfo->dwConnectionIndex );
		return false;
	}

	return g_pUserTable->DisconnectUserBySuggest( pUserInfo );
}


// 접속종료 대기자들을 위한 처리.
DWORD CUserTable::CloseConnectionWithAwaitingToDisconnect()
{
	DWORD dwTargetTick = IncreaseDisconnectTick();
	DWORD dwCount = 0;

	USERINFO_LIST* pUserInfoList = this->m_pAwaitingDisconnectUserList[dwTargetTick];
	USERINFO_LIST* pNextUserInfoList;

	while( pUserInfoList )
	{
		dwCount++;
		g_pINet->CompulsiveDisconnectUser(pUserInfoList->pUserInfo->dwConnectionIndex);

		pNextUserInfoList = pUserInfoList->pNextUserInfoList;

		pUserInfoList->pUserInfo->pAwaitingDisconnectUserInfoList = NULL;
		delete pUserInfoList;
		pUserInfoList = pNextUserInfoList;
	}
	this->m_pAwaitingDisconnectUserList[dwTargetTick] = NULL;

	return dwCount;
}


#ifdef __IS_AGENT_SERVER

// 오랜시간 메시지 전송이 없는 사용자들을 위한 처리.
void CUserTable::SetTickForSleptTimeProcess( USERINFO *pUserInfo )
{
	pUserInfo->dwSleepProcessTick = this->m_dwSleepProcessTick;

	return;
}

DWORD CUserTable::RemoveAllUserByMapServerConnectionIndex( DWORD dwMapServerConnectionIndex )
{
	USERINFO* cur = NULL;
	int counter = 0;

	for( DWORD i = 0; i<m_wMaxBucketNum; i++ )
	{
		if( (cur = m_ppInfoTable[i]) )
		{
			if( cur->dwMapServerConnectionIndex == dwMapServerConnectionIndex )
			{
				// 연결 끊기.
				g_pUserTable->DisconnectUserBySuggest( cur, CMD_CLOSE_CONNECTION_ABNORMAL );
				counter++;
			}
		}
	}
	MyLog( LOG_NORMAL, "Map Server Connection Lost :: Close User Connections(%d Users Are Closed)", counter );

	return counter;
}

DWORD CUserTable::RemoveAllUserByDBDemonConnectionIndex( DWORD dwDBDemonConnectionIndex )
{
	USERINFO* cur = NULL;
	int counter = 0;

	for( DWORD i = 0; i<m_wMaxBucketNum; i++ )
	{
		if( (cur = m_ppInfoTable[i]) )
		{
			if( cur->dwDBDemonConnectionIndex == dwDBDemonConnectionIndex )
			{
				// 연결 끊기.
				g_pINet->CompulsiveDisconnectUser( cur->dwConnectionIndex );
				counter++;
			}
		}
	}
	MyLog( LOG_NORMAL, "DB Demon Connection Lost :: Close User Connections(%d Users Are Closed)", counter );

	return counter;
}

DWORD CUserTable::CloseConnectionWithSleepingUsers( void )
{
	static dwCounter = 0;
	DWORD dwDisconnected = 0;
	USERINFO *pUser = NULL;

	if( (++dwCounter) == 15 )
		dwCounter = 0;

	// 1초마다 들어오므로 15초에 한번씩 진입하도록 한다.
	if( dwCounter == 0 )
	{
		this->m_dwSleepProcessTick++;
		if( this->m_dwSleepProcessTick >= MAX_ALLOWED_SLEEP_TIME )
			this->m_dwSleepProcessTick = 0;

		for( WORD i = 0; i < this->m_wMaxBucketNum; i++ )
		{
			pUser = this->m_ppInfoTable[i];
			while (pUser)
			{
				if( pUser->dwSleepProcessTick == m_dwSleepProcessTick )
				{
					MyLog( LOG_IGNORE, "User %s Connection CLOSED :: No packet exchange.", pUser->szName );
					dwDisconnected++;
					this->DisconnectUserBySuggest( pUser, CMD_CLOSE_CONNECTION_SLEPT_TOO_LONG_TIME );
				}
				pUser = pUser->pNextUserInfo;
			}
		}
	}

	return dwDisconnected;
}

#endif

bool CUserTable::SendToUser( DWORD dwUserID, char* pMsg, DWORD dwLength )
{
	USERINFO *pUserInfo = this->GetUserInfo( dwUserID );

	return this->SendToUser( pUserInfo, pMsg, dwLength );
}

bool CUserTable::SendToUser( USERINFO* pUserInfo, char* pMsg, DWORD dwLength )
{
	if( !pUserInfo || !pUserInfo->dwConnectionIndex )
		return false;
	if( pUserInfo->dwStatus != STATUS_USER_ACTIVATED )
		return false;

	return g_pINet->SendToUser( pUserInfo->dwConnectionIndex, pMsg, dwLength, FLAG_SEND_NOT_ENCRYPTION );
}

bool CUserTable::SendToUserByConnectionIndex( DWORD dwConnectionIndex, char* pMsg, DWORD dwLength )
{
	return g_pINet->SendToUser( dwConnectionIndex, pMsg, dwLength, FLAG_SEND_NOT_ENCRYPTION );
}

bool CUserTable::IsUserAvailable( DWORD dwUserID )
{
	USERINFO* pUserInfo = this->GetUserInfo( dwUserID );

	return this->IsUserAvailable( pUserInfo );
}

bool CUserTable::IsUserAvailable( USERINFO* pUserInfo )
{
	bool result;

	if( !pUserInfo )
		return false;

	switch( pUserInfo->dwStatus )
	{
	case STATUS_USER_ACTIVATED:
		{
			result = true;
		}
		break;
	case STATUS_USER_INACTIVATED:
	case STATUS_USER_AWAITING_DISCONNECT:
		{
			result = false;
		}
		break;
	default:
		{
			result = false;
		}
		break;
	}

	return result;
}

