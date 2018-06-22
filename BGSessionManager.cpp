#include "stdafx.h"
#include "BGSessionManager.h"


BGSessionManager::BGSessionManager()
{
}


BGSessionManager::~BGSessionManager()
{
}

bool BGSessionManager::Start()
{
	for (int i = 0; i < BG_SESSION_POOL_SIZE; i++)
	{
		BGSession * pSession{ nullptr };
		pSession = BGSession::Create(i);
		m_SessionArray[i] = pSession;
		m_UnusedSessionQueue.push(pSession);
	}

	BG_LOG_DEBUG("SessionManager Start Success");

	return true;
}

bool BGSessionManager::Stop()
{
	for (auto& session : m_SessionArray) {
		delete session;
		session = nullptr;
	}

	BG_LOG_DEBUG("SessionManager Stop Success");

	return true;
}

BGSession * BGSessionManager::GetSession(int index)
{
	if (index >= BG_SESSION_POOL_SIZE) {
		BG_LOG_ERROR("index >= BG_SESSION_POOL_SIZE");
		return nullptr;
	}

	return m_SessionArray[index];
}

/**
 * 세션 풀에서 사용가능한 세션을 하나 얻어옵니다.
 * 이때, 사용가능한 세션 풀에서 제거하기 때문에 이후에 반드시 AddSession을 호출해야 합니다.
 * 만약 사용하지 않으려면 ReturnSession을 호출해서 세션을 다시 반납해야 합니다.
*/
BGSession * BGSessionManager::GetSessionNew()
{	
	m_SessionQueueMutex.lock();

	if (m_UnusedSessionQueue.empty()) {
		BG_LOG_WARNING("Session Pool is Max");
		m_SessionQueueMutex.unlock();
		return nullptr;
	}

	auto pSession = m_UnusedSessionQueue.front();
	m_UnusedSessionQueue.pop();
	
	m_SessionQueueMutex.unlock();

	return pSession;
}

/** 
 * 세션을 사용가능한 세션 풀에 넣습니다.
*/
bool BGSessionManager::ReturnSession(BGSession * pSession)
{
	if (pSession == nullptr) {
		BG_LOG_ERROR("pSession is nullptr");
		return false;
	}

	m_SessionQueueMutex.lock();
	m_UnusedSessionQueue.push(pSession);
	m_SessionQueueMutex.unlock();

	return true;
}

bool BGSessionManager::AddSession(BGSession * pSession)
{
	if (pSession == nullptr) {
		BG_LOG_ERROR("pSession is nullptr");
		return false;
	}

	return true;
}

bool BGSessionManager::RemoveSession(BGSession * pSession)
{
	if (pSession == nullptr) {
		BG_LOG_ERROR("pSession is nullptr");
		return false;
	}

	// 세션을 반납하기 이전에 세션정보 초기화
	pSession->Reset();
	
	// 세션 반납
	if (false == ReturnSession(pSession)) {
		BG_LOG_ERROR("ReturnSession is fail");
		return false;
	}

	return true;
}
