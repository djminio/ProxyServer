// Proxy.h: interface for the CProxy class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PROXY_H__1F2FACA9_A9E0_467B_BFB9_CE37994E9A7D__INCLUDED_)
#define AFX_PROXY_H__1F2FACA9_A9E0_467B_BFB9_CE37994E9A7D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CProxy  
{
public:
	CProxy();
	~CProxy();
	DWORD	dwUserNum;
	DWORD	dwLastCheckTime;
	DWORD	dwFailtoAllocUserNum;
	DWORD	dwTotalLogUser;
	DWORD	dwTotalFailUser;
	DWORD	dwNowDisconnectionFlag;
	DWORD	dwSecDisconnection;
	DWORD	dwTime;
	HANDLE	hKeyEvent[10];
	BOOL	bStartAccept;
	BOOL	bLimitLoginTryPerSec;
	BYTE	bTryLoginThisSec;
	BOOL	bLimit;
	DWORD	dwDefaultMaxUser;
	DWORD	dwDefaultLimit;
	DWORD	dwMaxUser;
	BOOL	bLimitMaxUser;
};


extern CProxy* g_pProxy;
#endif // !defined(AFX_PROXY_H__1F2FACA9_A9E0_467B_BFB9_CE37994E9A7D__INCLUDED_)
