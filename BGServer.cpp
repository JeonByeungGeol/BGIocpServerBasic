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

	// WorkerThread ����
	for (int i = 0; i < BG_WORKER_THREAD_NUM; i++)
		workerThreads.push_back(RunSpawn());

	// AcceptThread ����
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
		
	// AcceptThread ����
	if (acceptThread == nullptr) {		
		BG_LOG_ERROR("AcceptThread is nullptr");
	}
	acceptThread->join();
	delete acceptThread;
	acceptThread = nullptr;

	// WorkerThread ����
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
	// Locale(��������) ���α׷��� ���� ������ ����ȭ�ϱ� ���� ����ϴ� "����/���"����
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

std::thread * BGServer::RunSpawn()
{
	return new std::thread{ [this] {this->Run(); } };
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

void BGServer::Run()
{
	BG_LOG_DEBUG("Worker Thread Start");
	while (true) {
		if (false == IsRunning())
			break;





	}
	BG_LOG_DEBUG("Worker Thread Exit");
}


