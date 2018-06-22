#pragma once
#pragma comment (lib, "ws2_32")

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

	

private:
	/** 현재 서버가 동작중인지 판단하기 위한 변수와 뮤텍스*/
	std::shared_mutex shared_mutex;
	bool m_bServerRunning;

	/** 접속을 처리할 전용 스레드 */
	std::thread acceptThread;

	/** IOCP Worker Thread */
	std::vector<std::thread*> workerThreads;
};

