#include "stdafx.h"
#include "BGServer.h"


BGServer::BGServer()
	: m_bServerRunning(false), m_MaxConcurrentUser(0), m_ConcurrentUser(0)
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

	if (false == g_SessionManager.Start()) {
		BG_LOG_ERROR("g_SessionManager.Start is failed");
		return false;
	}

	if (false == m_IOCPHandler.Start()) {
		BG_LOG_ERROR("CompletionPortHandler.Start is failed");
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

	// WorkerThread 종료 요청
	for (int i = 0; i < workerThreads.size(); i++) {
		OverlapEx* overlap_ex = new OverlapEx;
		ZeroMemory(overlap_ex, sizeof(OverlapEx));
		overlap_ex->operation = BG_OP_OFF;
		overlap_ex->recv_buf.buf = reinterpret_cast<CHAR*>(overlap_ex->socket_buf);
		overlap_ex->recv_buf.len = 0;
		PostQueuedCompletionStatus(m_IOCPHandler.GetIOCP(), 1, -1, reinterpret_cast<LPOVERLAPPED>(overlap_ex));
	}

	// WorkerThread 종료
	for (auto& thread : workerThreads) {
		if (thread == nullptr)
			BG_LOG_ERROR("WorkerThread is nullptr");
		thread->join();
		delete thread;
		thread = nullptr;
	}

	// IOHandler 종료
	if (false == m_IOCPHandler.Stop()) {
		BG_LOG_ERROR("CompletionPortHandler.Stop is failed");
		return false;
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

		int userCnt = InterlockedIncrement(&m_ConcurrentUser);
		if (userCnt > m_MaxConcurrentUser) {
			m_MaxConcurrentUser = userCnt;
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

	if (!pNewSession->IsState(ESessionState::BG_NONE)) {
		BG_LOG_ERROR("pNewSession's state is not NONE");
		return false;
	}
		
	pNewSession->SetState(ESessionState::BG_CONNECT);
	pNewSession->m_Data.m_socket = socket;

	m_IOCPHandler.OnConnect(pNewSession);

	pNewSession->m_Data.m_recv_overlap.operation = BG_OP_RECV;
	pNewSession->m_Data.m_recv_overlap.packet_size = 0;
	pNewSession->m_Data.previous_data = 0;

	DWORD flags = 0;
	int res = WSARecv(socket, &pNewSession->m_Data.m_recv_overlap.recv_buf
		, 1, NULL, &flags, &pNewSession->m_Data.m_recv_overlap.original_overlap, NULL);
	if (0 != res) {
		int error_no = WSAGetLastError();
		if (WSA_IO_PENDING != error_no) {
			BG_LOG_ERROR("WSARecv is error [error_no=%d]", error_no);
		}
	}
	return true;
}

void BGServer::Run()
{
	BG_LOG_DEBUG("Worker Thread Start");

	DWORD io_size, key;
	OverlapEx *overlap;
	BOOL result;

	while (true) {
		if (false == IsRunning())
			break;

		result = GetQueuedCompletionStatus(m_IOCPHandler.GetIOCP(), &io_size, &key,
			reinterpret_cast<LPOVERLAPPED*>(&overlap), INFINITE);
		if (FALSE == result) {
			BG_LOG_ERROR("GetQueuedCompletionStatus is failed");
		}
				
		if (0 == io_size) { // 통신 종료
			InterlockedDecrement(&m_ConcurrentUser);
		}

		if (overlap == nullptr) {
			BG_LOG_ERROR("overlap is nullptr [key=%d]", key);
			continue;
		}
		else if (BG_OP_OFF == overlap->operation) {
			break;
		}
		else if (BG_OP_RECV == overlap->operation) {
			auto pSession = g_SessionManager.GetSession(key);
			
			unsigned char *buf_ptr = overlap->socket_buf;
			int remained = io_size;

			// 패킷 조립
			while (0 < remained) {
				if (0 == pSession->m_Data.m_recv_overlap.packet_size)
					pSession->m_Data.m_recv_overlap.packet_size = buf_ptr[0];

				int required = pSession->m_Data.m_recv_overlap.packet_size - pSession->m_Data.previous_data;
				if (remained >= required) {
					memcpy(pSession->m_Data.packet + pSession->m_Data.previous_data,
						buf_ptr, required);
					// ProcessPAcket();
					remained -= required;
					buf_ptr += required;
					pSession->m_Data.m_recv_overlap.packet_size = 0;
					pSession->m_Data.previous_data = 0;
				}
				else {
					memcpy(pSession->m_Data.packet + pSession->m_Data.previous_data, buf_ptr, remained);
					pSession->m_Data.previous_data += remained;
					remained = 0;
					buf_ptr += remained;
				}
			}

			DWORD flags = 0;
			WSARecv(pSession->m_Data.m_socket,
				&pSession->m_Data.m_recv_overlap.recv_buf, BG_OP_RECV, NULL, &flags,
				reinterpret_cast<LPWSAOVERLAPPED>(&pSession->m_Data.m_recv_overlap), NULL);
		}
		else if (BG_OP_SEND == overlap->operation) {
			// ioSize하고 실제 보낸 크기 비교 후 소켓 접속 끊기
			delete overlap;
		}
		




	}
	BG_LOG_DEBUG("Worker Thread Exit");
}




