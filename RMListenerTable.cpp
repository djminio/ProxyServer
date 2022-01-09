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

	m_ppInfoTable = new RM_LISTENER_INFO*[num];		//m_ppInfoTable�� (�������� ������ �ּҰ�)�� �ּҰ��� ����Ų��.. 
	memset(m_ppInfoTable,0,sizeof(RM_LISTENER_INFO*)*num);
	return TRUE;
}

bool CRMListenerTable::Add(char* ip, BYTE type,  DWORD dwConnectionIndex)
{
	LP_SERVER_DATA pServerData;
	pServerData = g_pServerTable->GetServerData( dwConnectionIndex );
	
	RM_LISTENER_INFO* info = new RM_LISTENER_INFO;	

	memset(info, 0, sizeof(RM_LISTENER_INFO));		
	info->bConnectType = RM_TYPE_LISTENER;		//�Ű����� type���� ����� ����!!   �Ű����� type�� ����, ������ �����̴�.
	info->dwConnectionIndex = dwConnectionIndex;
//	info->dwServerType = pServerData->dwServerType;
	strcpy(info->szIP, ip);

	//���� �����϶� dwConnectionIndex�� �����Ƿ� �׶� DLL�� ��������..
	//type�� 1�϶��� dwConnectionIndex�� 0�� �ƴҰ��̴�..   bucketnum 1�϶��� �Ʒ� if���� ����� ���̴�.
	if(dwConnectionIndex)
	{
		g_pINet->SetServerInfo(dwConnectionIndex, info);
	}

	AddInfo(info, type);	//HashTable�� �߰�
	return true;
}

//type �� 0�̸� ���� ���� ����, 1�̸� ������ ���� 
void CRMListenerTable::AddInfo(RM_LISTENER_INFO* info, BYTE type)
{													
	if(type)
		m_wConnect++;
	else
		m_wNotConnect++;
	
	if (!m_ppInfoTable[ type ])		//m_ppInfoTable[index] �� ������ ������ ��ȣ ������.
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
	Remove(ip, 0);	//���� ���ѻ��¸� �ϳ� ����� 
	Add(ip,1, dwConnectionIndex);		//Connect�� ���·� Move
}

void CRMListenerTable::MoveToDisconnectStatus(char* ip)
{
	Remove(ip, 1);	//���� �ѻ��¸� �ϳ� ����� 
	Add(ip,0);		//Disconnect�� ���·� Move
}


//Table�� �Ȱ��� Data�� �ִ��� �������� Ȯ���Ѵ�...  �Ȱ��� Data�� ������ True, ������ FALSE
BOOL CRMListenerTable::CheckDuplicateIP(char* ip, BYTE type)
{
	RM_LISTENER_INFO* cur = NULL;
	RM_LISTENER_INFO* next = NULL;
	
	cur = m_ppInfoTable[type];		//0�� ���� ���� �����̴�...
	while (cur)
	{
		next = cur->pNextInfo;
		
		if(!strcmp(cur->szIP, ip))	//������ �ֽ�..
		{
			return TRUE;
		}
			
		cur = next;
	}

	return FALSE;
}
