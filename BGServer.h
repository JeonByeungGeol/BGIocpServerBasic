#pragma once
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
	bool IsRun();

	

private:
	std::shared_mutex shared_mutex;
	bool m_bServerRunning;
};

