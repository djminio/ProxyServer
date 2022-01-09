// -----------------------------
// Wrote by chan78 at 2001/03/16
// -----------------------------
#include "DrServerManager.h"
#include "Proxy.h"
//#include "servertable.cpp"

#ifdef __IS_PROXY_SERVER
bool AnswerToManager( LP_MANAGER_PACKET pPacket, DWORD dwLength )
{
	pPacket->h.bPTCL = PTCL_MANAGER_ANSWER;

	return g_pUserTable->SendToUser( pPacket->h.uTarget.dwTargetManagerID, (char *)pPacket, dwLength );
}
#else
bool AnswerToManager( LP_MANAGER_PACKET pPacket, DWORD dwLength )
{
	pPacket->h.bPTCL = PTCL_MANAGER_ANSWER;

	// 현재로선 프락시 서버가 하나뿐이므로.
	return g_pServerTable->SendToProxyServer( (char*)pPacket, dwLength );
}
#endif

// -------------------------------------------------------------------
// PROXY SERVER 용
// -------------------------------------------------------------------
#ifdef __IS_PROXY_SERVER
bool OnRecvAuthMsgFromManager( USERINFO *pUserInfo, LP_MANAGER_PACKET pPacket, DWORD dwLength )
{
	if( dwLength != sizeof( MANAGER_PACKET ) )
	{	
		TRACE( "=============> SIZE:%d  LENGTH : %d\n", dwLength, sizeof( MANAGER_PACKET ));
		return false;
	}	
	if( pPacket->h.bPTCL != PTCL_MANAGER_QUERY )
	{	
		return false;
	}	
	if( pPacket->h.wCMD != MANAGER_CMD_AUTH )
	{	
		return false;
	}	
//	if( pPacket->h.uTarget.wTargetServerID != g_pServerTable->GetOwnServerData()->wPort )
//	{	
//		return false;
//	}	
	if( strcmp(pPacket->b.MgrRequestAuthPacket.szID, MANAGER_ID) )
	{	
		return false;
	}	
	if( strcmp(pPacket->b.MgrRequestAuthPacket.szPasswd, MANAGER_PASSWD) )
	{	
		return false;
	}	
	
	const int availablenum = GetPrivateProfileInt( "server_manager", "num_of_available_manager_ip", 0, SERVER_MANAGER_INI_ );
	if( availablenum <= 0 ) return false;
		
	for (int i = 0; i < availablenum; i ++)
	{	
		char	sDummyIP[MM_IP_LENGTH];
		char	keyname[ FILENAME_MAX ];
		memset( sDummyIP, 0, MM_IP_LENGTH );
		sprintf( keyname, "manager_ip%d", i );
		GetPrivateProfileString( "server_manager", keyname, "", sDummyIP, sizeof(sDummyIP), SERVER_MANAGER_INI_ );
		if( !strcmp(pUserInfo->szIP, sDummyIP) ) goto SUCCESS__;
	}	
		
	return false;
		
SUCCESS__:
		
	return true;
}


// 20010508 ADD
void AnswerAuthPacket( USERINFO* pUserInfo)
{
	
	char szSendMsg[MM_MAX_PACKET_SIZE];
	LP_MANAGER_PACKET pSendPacket = (LP_MANAGER_PACKET)szSendMsg;

	// 미리 채워둔다. Answer 용.
	pSendPacket->h.bPTCL = PTCL_MANAGER_ANSWER;
	pSendPacket->h.wCMD = MANAGER_CMD_AUTH;
	pSendPacket->h.uTarget.dwTargetManagerID = pUserInfo->dwID;

	pSendPacket->b.MgrAnswerAuthPacket.wPort = g_pServerTable->GetOwnServerData()->wPort;
	if( !AnswerToManager( pSendPacket, sizeof(MANAGER_PACKET_HEADER) + sizeof(MGR_ANSWER_AUTH_PACKET) ) )
	{
		MyLog( LOG_NORMAL, "RESPONSE_FIRST_PACKET :: Failed To Answer" );
	}
}


bool OnRecvMsgFromManager( USERINFO *pUserInfo, LP_MANAGER_PACKET pPacket, DWORD dwLength )
{
	char szSendMsg[MM_MAX_PACKET_SIZE];
	LP_MANAGER_PACKET pSendPacket = (LP_MANAGER_PACKET)szSendMsg;

	// 미리 채워둔다. Answer 용.
	pSendPacket->h.bPTCL = PTCL_MANAGER_ANSWER;
	pSendPacket->h.uTarget.dwTargetManagerID = pUserInfo->dwID;

	// PROXY SERVER 인 경우엔 이 함수로 넘겨진 패킷의 타겟이 내가 아닐수도 있다.
	// 다른 서버로 리다이렉션.
	if( pPacket->h.uTarget.wTargetServerID != g_pServerTable->GetOwnServerData()->wPort )
	{
		// 내가 처리할 패킷이 아니다. Redirection.
		LP_SERVER_DATA pServerData = g_pServerTable->GetServerData( pPacket->h.uTarget.wTargetServerID );
		
		// 타겟 서버가 없거나 접속되어있지 않다.
		if( !pServerData || !pServerData->dwConnectionIndex )
		{
			if( !AnswerToManager( pSendPacket, sizeof(MANAGER_PACKET_HEADER) ) )
			{
				MyLog( LOG_IMPORTANT, "MANAGER_CMD_TARGET_NOT_AVAILABLE :: Failed To Answer(dwID: %d, TargetServer:%d)", pUserInfo->dwID, pPacket->h.uTarget );
				return true;
			}
		}
		else
		{
			if( !g_pServerTable->Send( pServerData->dwConnectionIndex, (char *)pPacket, dwLength ) )
			{
				MyLog( LOG_IMPORTANT, "Failed To Send 'PTCL_MANAGER_QUERY' to (%d/Port:%d)", pServerData->dwConnectionIndex, pServerData->wPort );
			}
		}
		return true;
	}

	// PROXY가 직접 처리할 것들.
	switch( pPacket->h.wCMD )
	{
	case MANAGER_CMD_REQUEST_SERVER_INFO:
		{
			pSendPacket->h.wCMD = MANAGER_CMD_REQUEST_SERVER_INFO;

			pSendPacket->b.MgrServerInfoPacket.bIsUserAcceptAllow = g_pServerTable->IsUserAcceptAllowed();
			pSendPacket->b.MgrServerInfoPacket.dwNumOfMaxUser = g_pProxy->dwMaxUser;
			pSendPacket->b.MgrServerInfoPacket.NumOfLimit = g_pProxy->bLimitLoginTryPerSec;

			if( !AnswerToManager( pSendPacket, sizeof(MANAGER_PACKET_HEADER) + sizeof(MGR_SERVER_INFO_PACKET) ) )
			{
				MyLog( LOG_NORMAL, "MANAGER_CMD_REQUEST_SERVER_INFO :: Failed To Answer" );
				break;
			}
		}
		break;
	case MANAGER_CMD_REQUEST_SERVER_SUBLIST:
		{
			DWORD i = 0;
			LP_SERVER_DATA pServerData;

			pSendPacket->h.wCMD = MANAGER_CMD_REQUEST_SERVER_SUBLIST;
			pSendPacket->b.MgrServerSubListPacket.wSubTotalNum = static_cast<WORD>(g_pServerTable->GetNumOfServers());

			for( pServerData = g_pServerTable->GetServerListHead(); pServerData; pServerData = pServerData->pNextServerData )
			{
				pSendPacket->b.MgrServerSubListPacket.wPort[i] = pServerData->wPort;
				i++;
			}

			if( !AnswerToManager( pSendPacket, sizeof(MANAGER_PACKET_HEADER) + sizeof(DWORD) + sizeof(WORD) * i) )
			{
				MyLog( LOG_NORMAL, "MANAGER_CMD_REQUEST_SERVER_SUBLIST :: Failed To Answer" );
				break;
			}

		}
		break;
	case MANAGER_CMD_REQUEST_SERVER_STATUS:
		{
			pSendPacket->h.wCMD = MANAGER_CMD_REQUEST_SERVER_STATUS;
			//ServerSet의 총 인원 
			pSendPacket->b.MgrServerStatusPacket.dwNumOfUsers = g_pServerTable->GetNumOfUsersInServerSet();
			
			
			pSendPacket->b.MgrServerStatusPacket.dwStatus = g_pServerTable->GetOwnServerData()->dwStatus;
			pSendPacket->b.MgrServerStatusPacket.wSubConnectionNum = static_cast<WORD>(g_pServerTable->GetNumOfConnectedServers());

			if( !AnswerToManager( pSendPacket, sizeof(MANAGER_PACKET_HEADER) +sizeof(MGR_SERVER_STATUS_PACKET) ) )
			{
				MyLog( LOG_NORMAL, "MANAGER_CMD_REQUEST_SERVER_STATUS :: Failed To Answer" );
				break;
			}
		}
		break;
	case MANAGER_CMD_REQUEST_SUBSERVER_INFO:
		{
			pSendPacket->h.wCMD = MANAGER_CMD_REQUEST_SUBSERVER_INFO;

			LP_SERVER_DATA pServerData = g_pServerTable->GetServerData( pPacket->b.MgrSubServerRequestPacket.wPort );

			if( pServerData != NULL )
			{
				pSendPacket->b.MgrSubServerInfoPacket.wType = pServerData->dwServerType;
				strcpy( pSendPacket->b.MgrSubServerInfoPacket.szIp, pServerData->szIP );

				if( !AnswerToManager( pSendPacket, sizeof(MANAGER_PACKET_HEADER) + sizeof(MGR_SUBSERVER_INFO_PACKET) ) )
				{
					MyLog( LOG_NORMAL, "MANAGER_CMD_REQUEST_SUBSERVER_INFO :: Failed To Answer" );
					break;
				}
			}
			else
			{
				MyLog( LOG_NORMAL, "MANAGER_CMD_REQUEST_SUBSERVER_INFO :: Failed To Answer" );
				break;
			}
		}
		break;
	
	//서브 서버 상태 요구(map, agent, db)
	case MANAGER_CMD_REQUEST_SUBSERVER_STATUS:
		{
			pSendPacket->h.wCMD = MANAGER_CMD_REQUEST_SUBSERVER_STATUS;
			
			LP_SERVER_DATA pServerData = g_pServerTable->GetServerData( pPacket->b.MgrSubServerRequestPacket.wPort );
			
			if( pServerData != NULL )
			{
				pSendPacket->b.MgrServerStatusPacket.dwStatus = pServerData->dwStatus;
				pSendPacket->b.MgrServerStatusPacket.dwNumOfUsers = pServerData->dwNumOfUsers;

				if( !AnswerToManager( pSendPacket, sizeof(MANAGER_PACKET_HEADER) + (sizeof(DWORD) * 2) ) )
				{
					MyLog( LOG_NORMAL, "MANAGER_CMD_REQUEST_SUBSERVER_STATUS :: Failed To Answer" );
					break;
				}
			}
			else
			{
				MyLog( LOG_NORMAL, "MANAGER_CMD_REQUEST_SUBSERVER_STATUS :: Failed To Answer" );
				break;
			}
		}
		break;

	case MANAGER_CMD_CONTROL_USER_ALLOW:
		{
			if( pPacket->b.MgrUserAllowPacket.bIsUserAcceptAllow == true )
			{
				if( g_pServerTable->IsUserAcceptAllowed() != true )
				{
					g_pServerTable->ToggleUserAcceptAllowed();
					MyLog( LOG_NORMAL, "USER ACCEPT ALLOWED by Key Event" );
				}
			}
			else
			{
				if( g_pServerTable->IsUserAcceptAllowed() == true )
				{
					g_pServerTable->ToggleUserAcceptAllowed();
					MyLog( LOG_NORMAL, "USER ACCEPT STOPED by Key Event" );
				}
			}		
		}
		break;
	case MANAGER_CMD_CONTROL_SERVER_MAXUSER:
		{
			g_pProxy->dwMaxUser = pPacket->b.MgrServerMaxUserPacket.dwNumOfMaxUser;
			MyLog( LOG_NORMAL, "MAX USER ADJUST: %d users can login this set. F3 -, F4 +",g_pProxy->dwMaxUser);
		}
		break;
	case MANAGER_CMD_CONTROL_SERVER_LIMITUSER:
		{
			g_pProxy->bLimitLoginTryPerSec = pPacket->b.MgrServerLimitUserPacket.NumOfLimit;
			MyLog( LOG_NORMAL, "Login Try LIMIT: %d users can login per sec now. F7 +, F8 -",g_pProxy->bLimitLoginTryPerSec);
		}
		break;
	case MANAGER_CMD_REQUEST_SERVER_SHUTDOWN:
		{
			if( pPacket->b.MgrServerShutDownPacket.wPort != g_pServerTable->GetOwnServerData()->wPort )
			{
				LP_SERVER_DATA pServerData = g_pServerTable->GetServerData( pPacket->b.MgrServerShutDownPacket.wPort );
				g_pServerTable->DestroyOtherServer( pServerData );
			}
			else
			{
				g_pServerTable->DestroyServer( FINISH_TYPE_NORMAL );
			}
		}
		break;
	case MANAGER_CMD_PUBLIC_NOTICE:
		{
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
		}
		break;
	default:
		{
			return false;
		}
	}
	return true;
}
#endif


// -------------------------------------------------------------------
// Agent SERVER 용
// -------------------------------------------------------------------
#ifdef __IS_AGENT_SERVER
bool OnRecvMsgFromManager( LP_MANAGER_PACKET pPacket, DWORD dwLength )
{
	char szSendMsg[MM_MAX_PACKET_SIZE];
	LP_MANAGER_PACKET pSendPacket = (LP_MANAGER_PACKET)szSendMsg;

	// 미리 채워둔다. Answer 용.
	pSendPacket->h.bPTCL = PTCL_MANAGER_ANSWER;
	pSendPacket->h.uTarget.dwTargetManagerID = pPacket->h.uTarget.dwTargetManagerID;

	switch( pPacket->h.wCMD )
	{
	case MANAGER_CMD_REQUEST_SERVER_STATUS:
		{
			pSendPacket->h.wCMD = MANAGER_CMD_REQUEST_SERVERSET_STATUS;
			pSendPacket->b.MgrServerStatusPacket.dwNumOfUsers = g_pUserTable->GetUserNum();
			pSendPacket->b.MgrServerStatusPacket.dwTotalServerConnections = g_pServerTable->GetNumOfConnectedServers();
			pSendPacket->b.MgrServerStatusPacket.dwStatus = g_pServerTable->GetOwnServerData()->dwStatus;
			sprintf( pSendPacket->b.MgrServerStatusPacket.szServerName, "AGENT SERVER(%d)", g_pServerTable->GetOwnServerData()->wPort );

			if( !AnswerToManager( pSendPacket, sizeof(MANAGER_PACKET) ) )
			{
				MyLog( LOG_NORMAL, "MANAGER_CMD_REQUEST_SERVER_STATUS :: Failed To Answer" );
				break;
			}
		}
		break;
	default:
		{
			return false;
		}
	}
	return true;
}
#endif


// -------------------------------------------------------------------
// Map SERVER 용
// -------------------------------------------------------------------
#ifdef __IS_MAP_SERVER
extern char MapName[ NM_LENGTH];	

bool OnRecvMsgFromManager( LP_MANAGER_PACKET pPacket, DWORD dwLength )
{
	char szSendMsg[MM_MAX_PACKET_SIZE];
	LP_MANAGER_PACKET pSendPacket = (LP_MANAGER_PACKET)szSendMsg;

	// 미리 채워둔다. Answer 용.
	pSendPacket->h.bPTCL = PTCL_MANAGER_ANSWER;
	pSendPacket->h.uTarget.dwTargetManagerID = pPacket->h.uTarget.dwTargetManagerID;

	switch( pPacket->h.wCMD )
	{
	case MANAGER_CMD_REQUEST_SERVER_STATUS:
		{
			pSendPacket->h.wCMD = MANAGER_CMD_REQUEST_SERVERSET_STATUS;
			pSendPacket->b.MgrServerStatusPacket.dwNumOfUsers = g_pServerTable->GetNumOfUsers();
			pSendPacket->b.MgrServerStatusPacket.dwTotalServerConnections = g_pServerTable->GetNumOfConnectedServers();
			pSendPacket->b.MgrServerStatusPacket.dwStatus = g_pServerTable->GetOwnServerData()->dwStatus;
			sprintf( pSendPacket->b.MgrServerStatusPacket.szServerName, "MAP SERVER(%d/%s)", g_pServerTable->GetOwnServerData()->wPort, MapName );

			if( !AnswerToManager( pSendPacket, sizeof(MANAGER_PACKET) ) )
			{
				MyLog( LOG_NORMAL, "MANAGER_CMD_REQUEST_SERVER_STATUS :: Failed To Answer" );
				break;
			}
		}
		break;
	default:
		{
			return false;
		}
	}
	return true;
}
#endif


// -------------------------------------------------------------------
// DB Demon 용
// -------------------------------------------------------------------
#ifdef __IS_DB_DEMON
bool OnRecvMsgFromManager( LP_MANAGER_PACKET pPacket, DWORD dwLength )
{
	char szSendMsg[MM_MAX_PACKET_SIZE];
	LP_MANAGER_PACKET pSendPacket = (LP_MANAGER_PACKET)szSendMsg;

	// 미리 채워둔다. Answer 용.
	pSendPacket->h.bPTCL = PTCL_MANAGER_ANSWER;
	pSendPacket->h.uTarget.dwTargetManagerID = pPacket->h.uTarget.dwTargetManagerID;

	switch( pPacket->h.wCMD )
	{
	case MANAGER_CMD_REQUEST_SERVER_STATUS:
		{
			pSendPacket->h.wCMD = MANAGER_CMD_REQUEST_SERVERSET_STATUS;
			pSendPacket->b.MgrServerStatusPacket.dwNumOfUsers = g_pServerTable->GetNumOfUsers();
			pSendPacket->b.MgrServerStatusPacket.dwTotalServerConnections = g_pServerTable->GetNumOfConnectedServers();
			pSendPacket->b.MgrServerStatusPacket.dwStatus = g_pServerTable->GetOwnServerData()->dwStatus;
			sprintf( pSendPacket->b.MgrServerStatusPacket.szServerName, "DB DEMON(%d)", g_pServerTable->GetOwnServerData()->wPort );

			if( !AnswerToManager( pSendPacket, sizeof(MANAGER_PACKET) ) )
			{
				MyLog( LOG_NORMAL, "MANAGER_CMD_REQUEST_SERVER_STATUS :: Failed To Answer" );
				break;
			}
		}
		break;
	default:
		{
			return false;
		}
	}
	return true;
}
#endif
