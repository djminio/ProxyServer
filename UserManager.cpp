// UserManager.cpp: implementation of the CUserManager class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "UserManager.h"

#pragma comment (lib,"winmm.lib")
#include <mmsystem.h>

///////////////////////////////////////////////////////////////////////////////
// Global Member
///////////////////////////////////////////////////////////////////////////////

CUserManager g_mgrUser;

///////////////////////////////////////////////////////////////////////////////
// Construction/Destruction
///////////////////////////////////////////////////////////////////////////////

CUserManager::CUserManager()
{
	m_mpLogin.clear();
	m_mpLogout.clear();
}

CUserManager::~CUserManager()
{
	m_mpLogin.clear();
	m_mpLogout.clear();
}

///////////////////////////////////////////////////////////////////////////////
// Public Method
///////////////////////////////////////////////////////////////////////////////

void CUserManager::AddLogin(const char* pName)
{
	const DWORD dwCurrentTime = ::timeGetTime();
	m_mpLogin.insert(PAIR_USER(pName, dwCurrentTime));
}
	
void CUserManager::DelLogin(const char* pName)
{
	ITOR_USER itor = m_mpLogin.find(pName);

	if (itor != m_mpLogin.end())
	{
		m_mpLogin.erase(itor);
	}
}

void CUserManager::AddLogout(const char* pName)
{
	const DWORD dwCurrentTime = ::timeGetTime();
	m_mpLogout.insert(PAIR_USER(pName, dwCurrentTime));
}

void CUserManager::DelLogout()
{
	for (ITOR_USER i = m_mpLogout.begin(); i != m_mpLogout.end(); )
	{
		//const DWORD dwFlowTime = ::timeGetTime() - i->second;

		//if (dwFlowTime >= DELAY)
		//{
			m_mpLogout.erase(i++);
		//}
		//else
		//{
		//	++i;
		//}
	}
}