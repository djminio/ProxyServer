#include "stdafx.h"
#include "inetwork.h"
#include "RMTable.h"
#include "ServerTable.h"
#include "Proxy.h"
#include "Proxy.h"
#include "DRServerManager.h"
#include "dr_agent_structures.h"

extern I4DyuchiNET* g_pINet;
extern CServerTable* g_pServerTable;
extern EchoBucket g_EchoBucket[ MAX_SERVER_NUM ];	//Echo 메세지 받을 Bucket
extern DWORD g_dwEchoID;

void ForceLogoffUser( const char* szUserID ); // 030224 kyo
void ForceLogoffUserAllStart();	// BBD 040110
void ForceLogoffUserSeveral();	// BBD 040110
void AllowedAgentJoin();		// BBD 040110

extern inline void MgrSend(DWORD dwConnectionIndex, void*pMsg, DWORD dwLength)//fixed compiling error with release mode. Reece- 17/09/2021 was causing a unknown symbol error added extern to keep the same format so it can easily find the proper symbol to compile client.
{
	//Modified by KBS 020330
	// Finito fixed...packet was being sent to user instead of server!!
	g_pINet->SendToServer(dwConnectionIndex, (char*)pMsg, dwLength, FLAG_SEND_NOT_ENCRYPTION);
}

inline BOOL CheckValidConnectionIndex(DWORD dwConnectionIndex)
{
	char szIp[16];	memset(szIp,0,16);	
	WORD wPort = 0;	
	
	g_pINet->GetServerAddress(dwConnectionIndex ,szIp,&wPort);
	//Modified by KBS 020330
	//g_pINet->GetUserAddress(dwConnectionIndex ,szIp,&wPort);
	if(!strcmp(szIp,"") && !wPort)
		return FALSE;
	else
		return TRUE;
}

void RMProc(DWORD dwConnectionIndex, char* pMsg, DWORD dwLength)
{
	BYTE header;
	memcpy(&header,pMsg+1, 1);

	static bool s_bKickOffState = false;	// BBD 040110	킥오프 상태를 RM에서 알기위해서

	switch(header)
	{
	//--------------------------------------------------------------------------------------------
	//									Tool( Client ) Packet
	//--------------------------------------------------------------------------------------------
	case MSG_RM_LOGIN:
		{
			
			PACKET_RM_LOGIN *packet = (PACKET_RM_LOGIN*)pMsg;

			if(g_pRMTable->CheckCertainIP(dwConnectionIndex, packet->IP))
			{
				//로그인 승인 메세지..   자신이 서버셋 넘버와 함께..
				PACKET_RM_LOGIN_OK pck(static_cast<BYTE>(((g_pServerTable->m_dwServerSetNumber))));
				MgrSend(dwConnectionIndex, &pck, pck.GetPacketSize());

			}
			else
			{
				//등록된 IP가 아닌자리에서 로그인 한 경우 
				PACKET_RM_LOGIN_FAIL pck;
				MgrSend(dwConnectionIndex, &pck, pck.GetPacketSize());
			}

			g_pRMTable->AddClient(dwConnectionIndex, packet);
			
			if(g_pRMTable->GetClientNum() == 1)	//RMClient가 하나라도 접속된 상태면 체크 시작!
			{
				StopWaitTimer();
				StartEchoTimer();			//서버 다운됐나 안됐나 체크 작업 시작
			}
		}
		break;


	case MSG_RM_REQUEST_ALL_SEBSERVER_INFO:
		{
			PACKET_REQUEST_ALL_SUBSERVER_INFO *packet = (PACKET_REQUEST_ALL_SUBSERVER_INFO*)pMsg;
			
			WORD wServerNum = static_cast<WORD>(g_pServerTable->GetNumOfServers());	// 자기 자신(Proxy)를 제외한 전체 Server 의 수.
			int nSize = 1/*header*/ + 1 + 1 + 4 + 4 + 2/*num*/ + (sizeof(ServerStatusInfo) * wServerNum); 
			//1 + 1 + 1 + 4 + 2 +(Server정보들).......

			char *pPacket = new char[ nSize ];
			int offset = 0;
			
			pPacket[0] = MSG_RM_REPLY_ALL_SEBSERVER_INFO;			//메세지 헤더 
			pPacket[1] = static_cast<char>(g_pServerTable->m_dwServerSetNumber);		//서버 셋트 넘버  
			pPacket[2] = packet->bOpenTemplate;						//이 메세지를 받았을때 새로운 서버 템플릿을 로드하느냐 마느냐 여부 
			memcpy(pPacket + 3, &packet->dwFrameID, sizeof(DWORD));	//메세지를 받을 FrameID
			memcpy(pPacket + 7, &g_pServerTable->m_dwNumOfUsersInServerSet, sizeof(DWORD));	//Proxy서버에 접속되어있는 총 유저수 
			memcpy(pPacket + 11, &wServerNum, sizeof(WORD));			//이 서버셋에 물려있는 총 서버갯수 
			offset = 13;

			LP_SERVER_DATA pServerData;
			for( pServerData = g_pServerTable->m_pServerListHead; pServerData; pServerData = pServerData->pNextServerData )
			{
				ServerStatusInfo info;
				info.wPort = pServerData->wPort;				//포트번호로 구분하기 위해 
				info.dwStatus = pServerData->dwStatus;			//서버 현재 상태 
				info.dwNumOfUsers = pServerData->dwNumOfUsers;	//접속 유저수 
			
				memcpy(pPacket + offset, &info, sizeof(info));
				offset += sizeof(info);
				//if( pServerData->wPort == wServerID )
				//{
				//	return pServerData;
				//}
			}	

			MgrSend(dwConnectionIndex, pPacket, offset);
			

			delete pPacket;
			
		}
		break;

	case MSG_RM_REQUEST_ALL_LISTENER_INFO:
		{
			PACKET_REQUEST_ALL_LISTENER_INFO *packet = (PACKET_REQUEST_ALL_LISTENER_INFO*)pMsg;

			WORD wListenerNum = g_pRMTable->m_ListenerTable.m_wConnect + g_pRMTable->m_ListenerTable.m_wNotConnect;
			
			int nSize = 8 + (wListenerNum * 5);
			DWORD ip;
			char *pPacket = new char[ nSize ];
			int offset = 0;

			pPacket[0] = MSG_RM_REPLY_ALL_LISTENER_INFO;			//메세지 헤더 
			pPacket[1] = static_cast<char>(g_pServerTable->m_dwServerSetNumber);		//서버 셋트 넘버  
			memcpy(pPacket + 2, &packet->dwFrameID, 4);	//메세지를 받을 FrameID
			memcpy(pPacket + 6, &wListenerNum, 2);					//리스터 갯수 
			offset = 8;

			RM_LISTENER_INFO* cur = NULL;
			RM_LISTENER_INFO* next = NULL;
			for(DWORD i=0; i<g_pRMTable->m_ListenerTable.m_dwMaxBucketNum; i++)
			{
				cur = g_pRMTable->m_ListenerTable.m_ppInfoTable[i];
				while (cur)
				{
					next = cur->pNextInfo;
					
					
					pPacket[ offset ] = i;			//Listener가 Proxy에 접속 되어있나 안되어있나 여부 1이면 접속 되있는것 0이면 접속 안되어있는것 
					offset += 1;
					ip = inet_addr(cur->szIP);

					memcpy(pPacket + offset, &ip, 4);
					offset += 4;
										
					cur = next;
				}
			}	

			MgrSend(dwConnectionIndex, pPacket, offset);

			delete pPacket;

		}
		break;

	case MSG_RM_CHECK_LISTENER_CONNECTION:
		{
			PACKET_CHECK_LISTENER_CONNECTION *packet = (PACKET_CHECK_LISTENER_CONNECTION*)pMsg;

			char ip[IP_LENGTH];	memset(ip,0,IP_LENGTH);	int count = 0;
			WORD port = 0;	
			DWORD dwTemp[ 30 ];	memset(dwTemp, 0, sizeof(dwTemp));
			
			RM_LISTENER_INFO* cur = NULL;
			RM_LISTENER_INFO* next = NULL;
			cur = g_pRMTable->m_ListenerTable.m_ppInfoTable[ 1 ];		//1이 접속 한 상태이니...
			while (cur)
			{
				next = cur->pNextInfo;
				
				g_pINet->GetServerAddress(cur->dwConnectionIndex ,ip,&port);
				if(!strcmp(ip,"") && !port)
				{
					for(int i=0; i<30; i++)
					{
						if(!dwTemp[i])
						{
							dwTemp[i] = cur->dwConnectionIndex;
							count++;
							break;
						}
					}
				}
				cur = next;
			}

			if(count > 0)
			{
				MyLog(LOG_NORMAL,"MSG_RM_CHECK_LISTENER_CONNECTION :: Pseudo index num %d",count);
			}

			PACKET_CHECK_LISTENER_CONNECTION_RESULT packet2(count);
			MgrSend(dwConnectionIndex, &packet2, packet2.GetPacketSize()); 
		}
		break;


	//지정한 서버로 ShutDown 메세지 날림 ..
	case MSG_RM_SHUTDOWN_SERVER:
		{
			BYTE bCount = pMsg[2];

			int offset = 3;
			WORD wPort;

			PACKET_SHUT_DOWN packet;

			for(int i=0; i<bCount; i++)
			{
				memcpy(&wPort,pMsg+offset,2);
				offset += 2;

				LP_SERVER_DATA pServerData;
				for( pServerData = g_pServerTable->m_pServerListHead; pServerData; pServerData = pServerData->pNextServerData )
				{
					if( pServerData->wPort == wPort )
					{
						g_pINet->SendToServer(pServerData->dwConnectionIndex ,
								(char*)&packet, sizeof(packet),FLAG_SEND_NOT_ENCRYPTION);

						break;
					}	
				}	
				
			}

			//g_pServerTable->DestroyServer( FINISH_TYPE_NORMAL );
		}
		break;

	case MSG_RM_USER_ACCEPT_ALLOWED:
		{
			if ( g_pServerTable->ToggleUserAcceptAllowed() == true )
			{
				MyLog( LOG_NORMAL, "USER ACCEPT ALLOWED by RMTool" );
			}
			else 
			{
				MyLog( LOG_NORMAL, "USER ACCEPT STOPED by RMTool" );
			}

		}
		break;


	case MSG_RM_SHUTDOWN_SUBSERVER:
		{
			PACKET_SHUTDOWN_SUBSERVER *rcvpacket = (PACKET_SHUTDOWN_SUBSERVER*)pMsg;

			PACKET_SHUT_DOWN packet;
			switch(rcvpacket->bServerType)
			{
			case 0:	//Proxy
				{
					g_pServerTable->DestroyServer( FINISH_TYPE_NORMAL );
				}
				break;

			case 1:	//Agent
				{
					g_pServerTable->BroadCastAgentServer((char*)&packet,2);
				}
				break;

			case 2:	//DBDemon
				{
					g_pServerTable->BroadCastDBDemon((char*)&packet,2);

				}
				break;

			case 3:	//MAP
				{
					g_pServerTable->BroadCastMapServer((char*)&packet,2);
				}
				break;

			}
		}
		break;


	case MSG_RM_REBOOT_SERVER:
		{
			int num = pMsg[2];
			int offset = 3;
			DWORD dwIndex = 0;
			WORD wPort;

			for(int i=0; i<num; i++)
			{
				memcpy(&wPort, pMsg+offset, 2);
				offset += 2;
				
				dwIndex = g_pRMTable->GetListenerConnectionIndex(wPort);
				if(!dwIndex)	continue;

				if(!CheckValidConnectionIndex(dwIndex))	
				{
					//Added by KBS 020110
					MyLog(LOG_NORMAL,"MSG_RM_REBOOT_SERVER :: Invalid Listener connection index(port: %d).", wPort);
					break;
				}
				
				PACKET_LISTENER_SERVER_REBOOT packet;
				g_pINet->SendToServer(dwIndex, (char*)&packet,packet.GetPacketSize(),FLAG_SEND_NOT_ENCRYPTION);
			}

		}
		break;


	case MSG_RM_CONNECT_ALL_LISTENER:
		{
			PACKET_CONNECT_ALL_LISTENER *packet = (PACKET_CONNECT_ALL_LISTENER*)pMsg;

			g_pRMTable->ConnectAllDisconnectedListener();
			MyLog( LOG_NORMAL, "Connected All Disconnect Listener by RMTool" );

			PACKET_LISTENER_CONNECT_COMPLETE packet2(packet->dwFrameID);
			MgrSend(dwConnectionIndex, &packet2, packet2.GetPacketSize());
		}
		break;

	case MSG_RM_PROXY_CONTROL:
		{
			PACKET_RM_PROXY_CONTROL *packet = (PACKET_RM_PROXY_CONTROL*)pMsg;

			g_pServerTable->m_bIsUserAcceptAllowed = packet->bUserAccept;

			//User Accept Control
			EnterCriticalSection( &g_pServerTable->m_IsUserAcceptAllowedCriticalSection );
			g_pServerTable->m_bIsUserAcceptAllowed = packet->bUserAccept;
			
			if( g_pServerTable->m_bIsUserAcceptAllowed == true )
				MyLog( LOG_NORMAL, "USER ACCEPT ALLOWED BY RMTool" );
			else
				MyLog( LOG_NORMAL, "USER ACCEPT STOPED BY RMTool" );
			LeaveCriticalSection( &g_pServerTable->m_IsUserAcceptAllowedCriticalSection );

			//Max User Control
			g_pProxy->dwMaxUser = packet->wMaxUser;
			MyLog( LOG_NORMAL, "MAX USER ADJUST By RMTool: %d users can login this set.",g_pProxy->dwMaxUser);

			//Try to Connect
			g_pProxy->bLimitLoginTryPerSec = packet->bTryToConnect;
			if (g_pProxy->bLimitLoginTryPerSec)
				MyLog( LOG_NORMAL, "Login Try LIMIT BY RMTool: %d users can login per sec now.",g_pProxy->bLimitLoginTryPerSec);
			else 
				MyLog( LOG_NORMAL, "Login Try UNLIMIT BY RMTool.");
		}
		break;


	case MSG_RM_EXECUTE_SERVER:
		{
			BYTE count, filelen, servertype;;
			WORD port;
			DWORD dwIndex;
			char szExeName[ MAX_PATH ];	memset(szExeName, 0, MAX_PATH);
			
			memcpy(&filelen, pMsg+2, 1);
			memcpy(szExeName, pMsg+3, filelen);

			memcpy(&count, pMsg+3+filelen, 1);
			int offset = 4+filelen;
			for(int i=0 ; i<count ; i++)
			{
				memcpy(&servertype, pMsg+offset, 1);
				offset += 1;

				memcpy(&port, pMsg+offset, 2);
				offset += 2;
				dwIndex = g_pRMTable->GetListenerConnectionIndex(port);
				if(!CheckValidConnectionIndex(dwIndex))	
				{
					//Added by KBS 020110
					MyLog(LOG_NORMAL,"MSG_RM_EXECUTE_SERVER :: Invalid Listener connection index(port: %d).", port);
					continue;
				}

				PACKET_LISTENER_SERVER_EXCUTE packet(servertype, port, szExeName);
				g_pINet->SendToServer(dwIndex, (char*)&packet,packet.GetPacketSize(),FLAG_SEND_NOT_ENCRYPTION);
			}

		}
		break;

	case MSG_RM_RELOAD_DATA:			//Reload Gameserver Data
		{
			BYTE count = pMsg[2];
			int offset = 3;
			WORD wPort;
			LP_SERVER_DATA pData;
			PACKET_RELOAD_GAMESERVER_DATA packet;
			for(int i=0; i<count; i++)
			{
				memcpy(&wPort, pMsg + offset, 2);
				pData = g_pServerTable->GetServerData(wPort);	
				offset += 2;
				
				if(!pData) continue;
				if(pData->dwConnectionIndex == 0)	continue;
					
				g_pINet->SendToServer(pData->dwConnectionIndex, (char*)&packet, packet.GetPacketSize(), FLAG_SEND_NOT_ENCRYPTION);

			}

		}
		break;

	case MSG_RM_NOTICE:
		{
			WORD wPort;
			LP_SERVER_DATA pServerData;
			//Length of Notice Message 
			WORD	msgLen;
			memcpy(&msgLen, pMsg + 2 , 2);
			
			//Notice Message
			char msg[ 2048 ];
			memcpy(msg, pMsg + 4, msgLen);
			msg[ msgLen ] = '\0';

			//Numer of Server
			BYTE num = pMsg[ msgLen + 4 ];
			int offset = msgLen + 5;

			for(int i=0; i< num; i++)
			{
				memcpy(&wPort, pMsg + offset, 2);
				pServerData = g_pServerTable->GetServerData(wPort);	
				offset += 2;
				
				if(!pServerData) continue;
				if(pServerData->dwConnectionIndex == 0)	continue;
					
				//BBS 패킷 전송부분 
				MANAGER_TO_MAP_PACKET PacketToMap;
				char dummy[2048];
				int len;

				//PacketToMap.h.header.type = CMD_BBS;
				PacketToMap.h.header.type = CMD_BBS_RMTOOL; //Eleval
				PacketToMap.h.header.size = msgLen;
				strncpy( PacketToMap.u.PublicNotice.msg, msg, msgLen );

				len = sizeof( PacketToMap.h ) + msgLen;

				dummy[0] = (BYTE)PTCL_BROADCAST_TO_SERVERS;
				memcpy( dummy+1, (char*)&PacketToMap, len );

				if( !g_pServerTable->Send( pServerData->dwConnectionIndex, (char *)dummy, len+1 ) )
				{
					MyLog( LOG_IMPORTANT, "Failed To Send For Public Notice" );
				}
				//

			}
		}
		break;

			
	case MSG_RM_CHANGE_WEATHER:
		{
			BYTE	bWeather	= pMsg[2];	//날씨 
			int		num			= pMsg[8];	//서버 갯수 
			BYTE	bStop		= pMsg[7];
			DWORD	dwAmount;
			memcpy(&dwAmount, pMsg+3, 4);
		
			int offset = 9;
			WORD wPort;
			LP_SERVER_DATA pData;
			
			for(int i=0; i<num; i++)
			{
				memcpy(&wPort, pMsg + offset, 2);
				pData = g_pServerTable->GetServerData(wPort);	
				offset += 2;
				
				if(!pData) continue;
				if(pData->dwConnectionIndex == 0)	continue;

				PACKET_CHANGE_WEATHER packet(bWeather, bStop, dwAmount);	
				g_pINet->SendToServer(pData->dwConnectionIndex, (char*)&packet, packet.GetPacketSize(), FLAG_SEND_NOT_ENCRYPTION);

			}
		}
		break;


// OtherServer -> Proxy

	case MSG_ECHO:
		{
			PACKET_ECHO *packet = (PACKET_ECHO*)pMsg;
			
			if(g_dwEchoID == packet->dwEchoID)
			{
				g_EchoBucket[ packet->bBucketIndex ].bReceived = 1;
			}
		}
		break;

			/*
			MANAGER_TO_MAP_PACKET PacketToMap;
			char dummy[612];
			int len;
			
			PacketToMap.h.header.type = CMD_BBS;
			PacketToMap.h.header.size = pPacket->b.MgrPublicNoticePacket.wLengthOfMsg;
			strncpy( PacketToMap.u.PublicNotice.msg, pPacket->b.MgrPublicNoticePacket.szMessage, PacketToMap.h.header.size );

			len = sizeof( PacketToMap.h ) + PacketToMap.h.header.size;

			dummy[0] = (BYTE)PTCL_BROADCAST_TO_SERVERS;
			memcpy( dummy+1, (char*)&PacketToMap, len );

			LP_SERVER_DATA pServerData = g_pServerTable->GetServerData( pPacket->b.MgrPublicNoticePacket.wPort );

			if( !g_pServerTable->Send( pServerData->dwConnectionIndex, (char *)dummy, len+1 ) )
			{
				MyLog( LOG_IMPORTANT, "Failed To Send For Public Notice" );
			}
			*/

	


/*
	//Tool에서 서버들 정보 요구 ~
	case MSG_RM_REQUEST_SERVER_STATUS:
		{
			LP_SERVER_DATA pServerData = g_pServerTable->GetServerData( pPacket->b.MgrSubServerRequestPacket.wPort );
			
			if( pServerData != NULL )
			{
				pSendPacket->b.MgrServerStatusPacket.dwStatus = pServerData->dwStatus;
				pSendPacket->b.MgrServerStatusPacket.dwNumOfUsers = pServerData->dwNumOfUsers;

				if( !AnswerToManager( pSendPacket, sizeof(MANAGER_PACKET_HEADER) + (sizeof(DWORD) * 2) ) )
				{
					MyLog( LOG_NORMAL, "MSG_RM_REQUEST_SERVER_STATUS :: Failed To Answer" );
					break;
				}
			}
			else
			{
				MyLog( LOG_NORMAL, "MSG_RM_REQUEST_SERVER_STATUS :: Failed To Answer" );
				break;
			}
		
		}
		break;
*/

	//--------------------------------------------------------------------------------------------
	//									Listener Packet
	//--------------------------------------------------------------------------------------------
	
	case MSG_RM_COPY_PATH:
	case MSG_RM_LISTENER_PATCH:
	case MSG_RM_DOWNLOAD_PATH:
		{
			g_pRMTable->BroadcastAllListener(pMsg,dwLength);		
		}
		break;

		
	
	
	case MSG_LISTENER_LOGIN:
		{

			// 쩝 이렇게 해서 얻는 서버 이름은 .. 속도가 너무 느려..  서버 부하가 된다..
			// 무식하더라도.. 로그인 패킷에 서버 이름을 보내자..
			/*
			in_addr addr;
			addr.S_un = g_pINet->GetServerAddress( dwConnectionIndex )->sin_addr.S_un;
			char ip[20];	memset(ip,0,20);
			memcpy(ip,inet_ntoa(addr),strlen(inet_ntoa(addr)));
			gethostbyaddr( ip,20,AF_INET); 
			*/
			//addr.S_un = g_pINet->GetUserAddress( info->ConnectionIndex )->sin_addr.S_un;
			//SetDlgItemText(hdlg,IDC_IPADDRESS,inet_ntoa(addr));
		}
		break;

	case MSG_RM_BROADCAST_AGENT:	
		{
			g_pServerTable->BroadCastAgentServer(pMsg,dwLength);
		}
		break;

	case MSG_RM_BROADCAST_DBDEMON:
		{
			g_pServerTable->BroadCastDBDemon(pMsg, dwLength);
		}
		break;

	case MSG_RM_BROADCAST_MAP:	
		{
			g_pServerTable->BroadCastMapServer(pMsg, dwLength);
		}
		break;

	case MSG_RM_KICKOFF_USER:	// 해당 유저를 Agent에서 바로 잘라버린다. 
		{
			ForceLogoffUser( (pMsg+2) );
			break;
		}
	//<! BBD 040110	RM으로 전제접속 종료시작 메시지
	case MSG_RM_KICKOFF_USER_ALL: 
		{
			// 유저로그인을 막는 루틴
//<! BBD 040401
			//User Accept Control
			if(!s_bKickOffState)
			{
				if ( g_pServerTable->ToggleUserAcceptAllowed() == false )
				{
					s_bKickOffState = true;
					ForceLogoffUserAllStart();		// 유저강퇴를 시작하라고 맵서버에 메시지를 날린다
					MyLog( LOG_NORMAL, "USER ACCEPT STOPED BY RMTool" );
					MyLog( LOG_NORMAL, "KickOff All User Started" );
					MyLog( LOG_NORMAL, "Start Blocking Agent -> Map Join" );
				}
			}
//> BBD 040401
			break;
		}
	//> BBD 040110	RM으로 전제접속 종료시작 메시지

	//<! BBD 040110 RM으로 일정 명수를 자르는 메시지
	case MSG_RM_KICKOFF_USER_SEVERAL:
		{
			if(s_bKickOffState)
				ForceLogoffUserSeveral();
			break;
		}
	//> BBD 040110 RM으로 일정 명수를 자르는 메시지
	
	//<! BBD 040110 현재의 Agent에서 맵서버로 조인이 가능토록 메시지를 뿌려준다
	case MSG_RM_KICKOFF_AGENTCANJOIN:
		{
//<! BBD 040401
			if(s_bKickOffState)
			{
				//User Accept Control
				if ( g_pServerTable->ToggleUserAcceptAllowed() == true )
				{
					s_bKickOffState = false;
					AllowedAgentJoin();
					MyLog( LOG_NORMAL, "USER ACCEPT BY RMTool" );
					MyLog( LOG_NORMAL, "KickOff All User End" );
					MyLog( LOG_NORMAL, "End Blocking Agent -> Map Join" );
				}
			}
//> BBD 040401
			break;
		}
	//> BBD 040110 현재의 Agent에서 맵서버로 조인이 가능토록 메시지를 뿌려준다
	}


}

void ForceLogoffUser( const char* szUserID ) // 030224 kyo
{
	PACKET_KICKOFF_USER packet;
	if( 0>=strlen( szUserID ) ) 
	{
		return;
	}
	strcpy( packet.szUserID, szUserID);

	LP_SERVER_DATA pServerData;
	for( pServerData = g_pServerTable->m_pServerListHead; pServerData; pServerData = pServerData->pNextServerData )
	{
		if( (pServerData->dwServerType == SERVER_TYPE_AGENT) && (pServerData->dwConnectionIndex) )//&& (pServerData->dwStatus == STATUS_ACTIVATED) )
		{
			g_pINet->SendToServer(pServerData->dwConnectionIndex ,
					(char*)&packet, sizeof(packet),FLAG_SEND_NOT_ENCRYPTION);

			break;
		}	
	}
}
//<! BBD 040110		전체 강퇴시작을 전체 맵서버에 보낸다
void ForceLogoffUserAllStart()
{
	PACKET_KICKOFF_USER_ALL packet;

	LP_SERVER_DATA pServerData;
	for( pServerData = g_pServerTable->m_pServerListHead; pServerData; pServerData = pServerData->pNextServerData )
	{
		if( (pServerData->dwServerType == SERVER_TYPE_MAP) && (pServerData->dwConnectionIndex) )//&& (pServerData->dwStatus == STATUS_ACTIVATED) )
		{
			g_pINet->SendToServer(pServerData->dwConnectionIndex ,
					(char*)&packet, sizeof(packet),FLAG_SEND_NOT_ENCRYPTION);

		}	
	}
}
//> BBD 040110		전체 강퇴시작을 전체 맵서버에 보낸다

//<! BBD 040110		맵서버에 일정 명수의 강퇴메시지를 보낸다
void ForceLogoffUserSeveral()
{
	PACKET_KICKOFF_USER_SEVERAL packet;

	LP_SERVER_DATA pServerData;
	for( pServerData = g_pServerTable->m_pServerListHead; pServerData; pServerData = pServerData->pNextServerData )
	{
		if( (pServerData->dwServerType == SERVER_TYPE_MAP) && (pServerData->dwConnectionIndex) )//&& (pServerData->dwStatus == STATUS_ACTIVATED) )
		{
			g_pINet->SendToServer(pServerData->dwConnectionIndex ,
					(char*)&packet, sizeof(packet),FLAG_SEND_NOT_ENCRYPTION);

		}	
	}
}
//> BBD 040110		맵서버에 일정 명수의 강퇴메시지를 보낸다

//<! BBD 040110		에이전트에서 맵서버로 조인 허용
void AllowedAgentJoin()
{
	PACKET_KICKOFF_AGENTCANJOIN packet;

	LP_SERVER_DATA pServerData;
	for( pServerData = g_pServerTable->m_pServerListHead; pServerData; pServerData = pServerData->pNextServerData )
	{
		if( (pServerData->dwServerType == SERVER_TYPE_MAP) && (pServerData->dwConnectionIndex) )//&& (pServerData->dwStatus == STATUS_ACTIVATED) )
		{
			g_pINet->SendToServer(pServerData->dwConnectionIndex ,
					(char*)&packet, sizeof(packet),FLAG_SEND_NOT_ENCRYPTION);

		}	
	}
}
//> BBD 040110		에이전트에서 맵서버로 조인 허용