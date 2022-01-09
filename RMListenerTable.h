//////////////////////////////////////////////////////////////////////////////////////
//				  CRMConnectTable Class by Byung-soo Koo		                    //
//																					//
//                                                      Last Update: 2001.11.09		//
//////////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "stdafx.h"
#include "RMDefine.h"


class CRMListenerTable
{

	
	
	void					RemoveAllTable();
	void					AddInfo(RM_LISTENER_INFO* info, BYTE type);
	
public:								  
	DWORD					m_dwMaxBucketNum;
	RM_LISTENER_INFO**		m_ppInfoTable;
	WORD					m_wConnect;								//접속에 성공한 Listner수 
	WORD					m_wNotConnect;							//아직 접속하지 못한 Listener..
	RM_LISTENER_INFO*		GetInfo(char *ip, BYTE type);
	bool					Add(char* ip, BYTE type, DWORD dwConnectionIndex = 0);
	void					Remove(char *ip, BYTE type);
	void					MoveToConnectStatus(char *ip, DWORD dwConnectionIndex);
	void					MoveToDisconnectStatus(char *ip);
	BOOL					CheckDuplicateIP(char* ip, BYTE type);
	BOOL					Create(DWORD num);
	
	CRMListenerTable();
	~CRMListenerTable();
};


