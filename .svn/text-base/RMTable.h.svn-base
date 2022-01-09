//////////////////////////////////////////////////////////////////////////////////////
//						  CRMTable Class by Byung-soo Koo		                    //
//																					//
//                                                      Last Update: 2001.11.06		//
//////////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "stdafx.h"
#include "RMDefine.h"
#include "RMListenerTable.h"

//RMTool, RMListenr의 접속 정보를 가지고 있을 HashTable로 
//CRMTable 자체는 Client(툴) 정보를 가지고 있고..
//CRMTable에 있는 멤버 변수인 m_ListenerTable이 Listener들의 정보를 가지고 있다. 
class CRMTable
{
	DWORD					m_dwMaxBucketNum;
	RMCLIENT_INFO**			m_ppInfoTable;
	int						m_CertainIPNum;
	char					m_RMCertIP[ MAX_RM_LOGIN ][16];
		
	void					RemoveAllClientTable();
	void					AddClientInfo(RMCLIENT_INFO* info);
	
public:								  
	CRMListenerTable		m_ListenerTable;			//Listener만을 관리하는 hash table
	
	BYTE					m_bClientConnect;			//RM Tool(Client)의 접속수
	RMCLIENT_INFO*			GetClientInfo(DWORD id);
	BYTE					GetClientNum();				//접속한 클라이언트 숫자를 리턴 
	bool					AddClient(DWORD dwConnectionIndex, PACKET_RM_LOGIN* packet);
	void					RemoveClientID(DWORD id);
	void					RemoveClient(DWORD dwConnectionIndex);
	void					ConnectAllDisconnectedListener();
	BOOL					BroadcastAllListener(char *packet, DWORD dwLength);
	BOOL					GetCertainIPFromIni();
	BOOL					CheckCertainIP(DWORD dwConnectionIndex, char* ip);
//	BOOL					BroadcastEachListener(DWORD dwServerType, char *packet, DWORD dwLength);	//서버 종류마다 Listener에게 메세지 전달.. 예)SERVER_TYPE_DB
	void					BroadcastAllRMClient(char* pMsg, DWORD dwLength);		
	BOOL					CheckExistIP(char *szIp);	//해당 IP가 Listener테이블에(접속이건 아니건) 있는지 없는지 체크 
	DWORD					GetListenerConnectionIndex( WORD wGameServerPort ); //게임 서버의 Port번호로 Listener의 ConnectionIndex를 받아냄 


	CRMTable(DWORD num);
	~CRMTable();
};

extern CRMTable* g_pRMTable;

extern void StartEchoTimer();
extern void StopEchoTimer();
extern void StartWaitTimer();
extern void StopWaitTimer();