#include "stdafx.h"
#include "BGIOCompletionHandler.h"


BGIOCompletionHandler::BGIOCompletionHandler()
{
}


BGIOCompletionHandler::~BGIOCompletionHandler()
{
}

bool BGIOCompletionHandler::Start()
{
	m_hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	
	return true;
}

bool BGIOCompletionHandler::Stop()
{
	if (m_hIOCP)
		CloseHandle(m_hIOCP);

	return true;
}

HANDLE BGIOCompletionHandler::GetIOCP()
{
	return m_hIOCP;
}

void BGIOCompletionHandler::OnConnect(void * p)
{
	BGSession* pSession = reinterpret_cast<BGSession*>(p);
	CreateIoCompletionPort((HANDLE)pSession->m_Data.m_socket, m_hIOCP, (DWORD)pSession->GetIndex(), 0);
}
