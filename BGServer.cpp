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
	BG_LOG_DEBUG("SERVER ON");

	if (false == Initialize()) {
		BG_LOG_ERROR("Initialize is failed");
		return false;
	}

	if (!g_SessionManager.Start()) {
		BG_LOG_ERROR("g_SessionManager.Start failed");
		return false;
	}

	shared_mutex.lock();
	m_bServerRunning = true;
	shared_mutex.unlock();

	if (false == ListenStart()) {
		BG_LOG_ERROR("ListenStart() is failed");
		return false;
	}

	// WorkerThread 동작
	for (int i = 0; i < BG_WORKER_THREAD_NUM; i++)
		workerThreads.push_back(RunSpawn());

	// AcceptThread 동작
	acceptThread = AcceptSpawn();




	

	BG_LOG_DEBUG("SERVER RUNNING");

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
		
	// AcceptThread 종료
	if (INVALID_SOCKET == closesocket(m_ListenSocket)) 
	{
		BG_LOG_ERROR("Close ListenSocket is failed");
		return false;
	}
	if (acceptThread == nullptr) {		
		BG_LOG_ERROR("AcceptThread is nullptr");
	}
	acceptThread->join();
	delete acceptThread;
	acceptThread = nullptr;

	// WorkerThread 종료
	for (auto& thread : workerThreads) {
		if (thread == nullptr)
			BG_LOG_ERROR("WorkerThread is nullptr");
		thread->join();
		delete thread;
		thread = nullptr;
	}


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

bool BGServer::ListenStart()
{
	m_ListenSocket = WSASocket(AF_INET, SOCK_STREAM,
		IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);

	struct sockaddr_in listen_addr;	
	ZeroMemory(&listen_addr, sizeof(listen_addr));
	listen_addr.sin_family = AF_INET;
	listen_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	listen_addr.sin_port = htons(g_Config.GetInt("MY_SERVER_PORT"));
	ZeroMemory(&listen_addr.sin_zero, 8);

	if (SOCKET_ERROR == ::bind(m_ListenSocket, reinterpret_cast<sockaddr*>(&listen_addr), sizeof(listen_addr))) {
		BG_LOG_ERROR("::bind() is failed");
		return false;
	}
	
	if (SOCKET_ERROR == listen(m_ListenSocket, 10)) {
		BG_LOG_ERROR("listen() is failed");
		return false;
	}

	return true;
}

std::thread* BGServer::AcceptSpawn()
{
	return new std::thread{ [this] { this->Accept(); } };
}

std::thread * BGServer::RunSpawn()
{
	return new std::thread{ [this] {this->Run(); } };
}

void BGServer::Accept()
{
	BG_LOG_DEBUG("Accept Thread Start");

	struct sockaddr_in client_addr;
	int addr_size = sizeof(client_addr);

	while (true) {
		if (false == IsRunning())
			break;

		SOCKET newClient = WSAAccept(m_ListenSocket, reinterpret_cast<sockaddr*>(&client_addr), &addr_size, NULL, NULL);
		
		if (newClient == INVALID_SOCKET && WSAGetLastError() == WSAEWOULDBLOCK) {
			BG_LOG_TRACE("newClient is INVALID_SOCKET or WSAEWOULDBLOCK");
			continue;
		}
		
		if (false == IsRunning())
			break;

			
		// 접속 처리
		if (false == AcceptProcess(newClient)) {
			BG_LOG_ERROR("AcceptProcess() is failed");
			continue;
		}
	}
	BG_LOG_DEBUG("Accept Thread Exit");
}

bool BGServer::AcceptProcess(SOCKET& socket)
{
	BGSession* pNewSession = g_SessionManager.GetSessionNew();
	if (nullptr == pNewSession) {
		BG_LOG_ERROR("pNewSession is nullptr");
		return false;
	}


	return true;
}

void BGServer::Run()
{
	BG_LOG_DEBUG("Worker Thread Start");
	while (true) {
		if (false == IsRunning())
			break;





	}
	BG_LOG_DEBUG("Worker Thread Exit");
}




