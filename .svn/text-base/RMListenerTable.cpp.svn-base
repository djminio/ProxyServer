#include "inetwork.h"
#include "RMListenerTable.h"

extern I4DyuchiNET *g_pINet;

CRMListenerTable::CRMListenerTable()
{
	m_wConnect = 0;
	m_wNotConnect = 0;
}

CRMListenerTable::~CRMListenerTable()
{
	RemoveAllTable();

	if (m_ppInfoTable)
	{
		delete [] m_ppInfoTable;
		m_ppInfoTable = NULL;
	}
}

BOOL CRMListenerTable::Create(DWORD num)
{
	if(num < 1)
		return FALSE;

	m_dwMaxBucketNum = num;

	m_ppInfoTable = NULL;

	m_ppInfoTable = new RM_LISTENER_INFO*[num];		//m_ppInfoTable은 (유저인포 내용의 주소값)의 주소값을 가르킨당.. 
	memset(m_ppInfoTable,0,sizeof(RM_LISTENER_INFO*)*num);
	return TRUE;
}

bool CRMListenerTable::Add(char* ip, BYTE type,  DWORD dwConnectionIndex)
{
	LP_SERVER_DATA pServerData;
	pServerData = g_pServerTable->GetServerData( dwConnectionIndex );
	
	RM_LISTENER_INFO* info = new RM_LISTENER_INFO;	

	memset(info, 0, sizeof(RM_LISTENER_INFO));		
	info->bConnectType = RM_TYPE_LISTENER;		//매개변수 type과는 헷깔리지 말것!!   매개변수 type은 접속, 미접속 여부이다.
	info->dwConnectionIndex = dwConnectionIndex;
//	info->dwServerType = pServerData->dwServerType;
	strcpy(info->szIP, ip);

	//접속 상태일때 dwConnectionIndex가 나오므로 그때 DLL에 셋팅하자..
	//type이 1일때만 dwConnectionIndex가 0이 아닐것이다..   bucketnum 1일때만 아래 if문이 수행될 것이다.
	if(dwConnectionIndex)
	{
		g_pINet->SetServerInfo(dwConnectionIndex, info);
	}

	AddInfo(info, type);	//HashTable에 추가
	return true;
}

//type 이 0이면 접속 안한 상태, 1이면 접속한 상태 
void CRMListenerTable::AddInfo(RM_LISTENER_INFO* info, BYTE type)
{													
	if(type)
		m_wConnect++;
	else
		m_wNotConnect++;
	
	if (!m_ppInfoTable[ type ])		//m_ppInfoTable[index] 에 내용이 없으면 괄호 안으로.
	{
		m_ppInfoTable[ type ] = info;
		info->pNextInfo = NULL;
		info->pPrevInfo = NULL;
		return;
	}


	RM_LISTENER_INFO* cur = m_ppInfoTable[ type ];
	RM_LISTENER_INFO* prv = NULL;
	while (cur)
	{
		prv = cur;
		cur = cur->pNextInfo;
	}
	
	cur = prv->pNextInfo = info;
	cur->pPrevInfo = prv;
	cur->pNextInfo = NULL;
}


RM_LISTENER_INFO* CRMListenerTable::GetInfo(char *ip, BYTE type)
{
	RM_LISTENER_INFO* cur = m_ppInfoTable[ type ];

	while (cur)
	{
		if (!strcmp(ip,cur->szIP))
		{
			return cur;
		}
		cur = cur->pNextInfo;
	}
	return NULL;
}

void CRMListenerTable::Remove(char *ip, BYTE type)
{

	RM_LISTENER_INFO* cur  = m_ppInfoTable[ type ];
	RM_LISTENER_INFO* next = NULL;
	RM_LISTENER_INFO* prv  = NULL;
	
	while (cur)
	{
		prv = cur->pPrevInfo;
		next = cur->pNextInfo;
		if (!strcmp(ip, cur->szIP))
		{
			if (!prv)	// if head
				m_ppInfoTable[type] = next;	
			else 
				prv->pNextInfo = next;
			
			if (next)
				next->pPrevInfo = prv;

			if(cur)	
				delete cur;		

			cur = NULL;

			if(type)
				m_wConnect--;
			else
				m_wNotConnect--;
			
			return;
		}
		cur = cur->pNextInfo;
	}
	return;
}


void CRMListenerTable::RemoveAllTable()
{
	RM_LISTENER_INFO* cur = NULL;
	RM_LISTENER_INFO* next = NULL;
	for (DWORD i=0; i<m_dwMaxBucketNum; i++)
	{
		cur = m_ppInfoTable[i];
		while (cur)
		{
			next = cur->pNextInfo;
			delete cur;
			cur = next;
		}
		m_ppInfoTable[i] = NULL;
	}

	m_wConnect = 0;
	m_wNotConnect = 0;
}

void CRMListenerTable::MoveToConnectStatus(char* ip, DWORD dwConnectionIndex)
{
	Remove(ip, 0);	//접속 안한상태를 하나 지우고 
	Add(ip,1, dwConnectionIndex);		//Connect한 상태로 Move
}

void CRMListenerTable::MoveToDisconnectStatus(char* ip)
{
	Remove(ip, 1);	//접속 한상태를 하나 지우고 
	Add(ip,0);		//Disconnect한 상태로 Move
}


//Table에 똑같은 Data가 있는지 없는지를 확인한다...  똑같은 Data가 있으면 True, 없으면 FALSE
BOOL CRMListenerTable::CheckDuplicateIP(char* ip, BYTE type)
{
	RM_LISTENER_INFO* cur = NULL;
	RM_LISTENER_INFO* next = NULL;
	
	cur = m_ppInfoTable[type];		//0이 접속 안한 상태이니...
	while (cur)
	{
		next = cur->pNextInfo;
		
		if(!strcmp(cur->szIP, ip))	//같은게 있슴..
		{
			return TRUE;
		}
			
		cur = next;
	}

	return FALSE;
}
