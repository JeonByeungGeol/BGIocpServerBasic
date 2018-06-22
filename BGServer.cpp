#include "stdafx.h"
#include "BGServer.h"


BGServer::BGServer()
	: m_bServerRunning(false)
{
}


BGServer::~BGServer()
{
}

bool BGServer::Start()
{
	if (false == Initialize()) {
		BG_LOG_ERROR("Initialize is Fail!");
		return false;
	}


	shared_mutex.lock();
	m_bServerRunning = true;
	shared_mutex.unlock();

	BG_LOG_DEBUG("SERVER ON");

	return true;
}

bool BGServer::Stop()
{
	if (IsRunning()) {
		BG_LOG_ERROR("Already server stop");
		return false;
	}

	shared_mutex.lock();
	m_bServerRunning = false;
	shared_mutex.unlock();

	BG_LOG_DEBUG("SERVER OFF");

	return true;
}

bool BGServer::Initialize()
{
	return true;
}

bool BGServer::IsRunning()
{
	shared_mutex.lock_shared();
	bool isRun = m_bServerRunning;
	shared_mutex.unlock_shared();

	return isRun;
}


