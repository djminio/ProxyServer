// Proxy.cpp: implementation of the CProxy class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Proxy.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CProxy* g_pProxy;

CProxy::CProxy()
{
	memset(this,0,sizeof(CProxy));
	dwUserNum				=	0;
	dwLastCheckTime			=	60;
	dwFailtoAllocUserNum	=	0;
	dwTotalLogUser			=	0;
	dwTotalFailUser			=	0;
	dwNowDisconnectionFlag	=	0;
	dwSecDisconnection		=	5;
	dwTime					=	0;
	bStartAccept			=	TRUE;
	bLimitLoginTryPerSec	=	0;
	bTryLoginThisSec		=	0;
	bLimit					=	1;
	dwDefaultMaxUser		=	0;
	dwDefaultLimit			=	0;
	bLimitMaxUser			=	1;
}

CProxy::~CProxy()
{

}
