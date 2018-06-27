#pragma once
#pragma comment (lib, "ws2_32")

#include "BGIOCompletionHandler.h"

class BGServer
{
public:
	BGServer();
	~BGServer();

	/** 서버를 시작합니다.*/
	bool Start();

	/** 서버를 종료합니다.*/
	bool Stop();

	/** 서버를 초기화합니다.*/
	bool Initialize();

	/** 서버가 동작중인지 확인합니다.*/
	bool IsRunning();
		
	/** 클라이언트로부터 접속을 처리하는 작업을 시작합니다.*/
	bool ListenStart();

	/** 클라이언트로부터 접속요청을 받을 수 있게 하는 함수*/
	void Accept();

	/** 클라이언트의 접속요청을 처리하는 작업입니다.*/
	bool AcceptProcess(SOCKET&);

	/** 패킷을 조립해서 처리하는 스레드함수입니다.*/
	void Run();	

private:
	/** Accept함수를 스레드에서 동작하기 위한 함수*/
	std::thread* AcceptSpawn();

	/** Run함수를 스레드에서 동작하기 위한 함수*/
	std::thread* RunSpawn();

private:
	/** 현재 서버가 동작중인지 판단하기 위한 변수와 뮤텍스*/
	std::shared_mutex shared_mutex;
	bool m_bServerRunning;

	/** 접속을 처리할 전용 스레드 */
	std::thread* acceptThread;

	/** IOCP Worker Thread */
	std::vector<std::thread*> workerThreads;

	/** 클라이언트 접속을 받을 소켓*/
	SOCKET m_ListenSocket;

	BGIOCompletionHandler m_IOCPHandler;
};

