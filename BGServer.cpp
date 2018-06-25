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

	if (!g_SessionManager.Start()) {
		BG_LOG_ERROR("g_SessionManager.Start failed");
		return false;
	}

	shared_mutex.lock();
	m_bServerRunning = true;
	shared_mutex.unlock();


	acceptThread = AcceptSpawn();




	

	BG_LOG_DEBUG("SERVER ON");

	return true;
}

bool BGServer::Stop()
{
	if (false == IsRunning()) {
		BG_LOG_ERROR("Already server stop");
		return false;
	}

	shared_mutex.lock();
	m_bServerRunning = false;
	shared_mutex.unlock();
		
	if (acceptThread == nullptr) {		
		BG_LOG_ERROR("AcceptThread is nullptr");
	}
	acceptThread->join();
	delete acceptThread;
	acceptThread = nullptr;



	if (!g_SessionManager.Stop()) {
		BG_LOG_ERROR("g_SessionManager.Stop failed");
		return false;
	}

	BG_LOG_DEBUG("SERVER OFF");

	return true;
}

bool BGServer::Initialize()
{
	// Locale(로케일은) 프로그램을 언어와 국가에 최적화하기 위해 사용하는 "지역/언어"정보
	_wsetlocale(LC_ALL, _T("korea"));
	
	WSADATA wsadata;
	WSAStartup(MAKEWORD(2, 2), &wsadata);

	
	return true;
}

bool BGServer::IsRunning()
{
	shared_mutex.lock_shared();
	bool isRun = m_bServerRunning;
	shared_mutex.unlock_shared();

	return isRun;
}

std::thread* BGServer::AcceptSpawn()
{
	return new std::thread{ [this] { this->Accept(); } };
}

void BGServer::Accept()
{
	BG_LOG_DEBUG("Accept Thread Start");
	while (true) {
		if (false == IsRunning())
			break;
		





	}
	BG_LOG_DEBUG("Accept Thread Exit");
}


