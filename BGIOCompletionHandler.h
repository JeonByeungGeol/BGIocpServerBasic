#pragma once
class BGIOCompletionHandler
{
public:
	BGIOCompletionHandler();
	~BGIOCompletionHandler();

	/** 객체 사용을 시작합니다.*/
	bool Start();

	/** 객체 사용을 종료합니다.*/
	bool Stop();

	/** IOCP 핸들을 얻습니다.*/
	HANDLE GetIOCP();

	/** IOCP 핸들에 연결합니다.*/
	void OnConnect(BGSession*);

private:
	HANDLE m_hIOCP;
};

