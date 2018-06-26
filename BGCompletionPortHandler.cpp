#include "stdafx.h"
#include "BGCompletionPortHandler.h"


BGCompletionPortHandler::BGCompletionPortHandler()
{
}


BGCompletionPortHandler::~BGCompletionPortHandler()
{
}

bool BGCompletionPortHandler::Start()
{
	m_hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	
	return true;
}

bool BGCompletionPortHandler::Stop()
{
	if (m_hIOCP)
		CloseHandle(m_hIOCP);

	return false;
}

void BGCompletionPortHandler::OnConnect(BGSession * pSession)
{
	CreateIoCompletionPort((HANDLE)pSession->m_Data.m_socket, m_hIOCP, (DWORD)pSession->GetIndex(), 0);
}
