#pragma once
class BGCompletionPortHandler
{
public:
	BGCompletionPortHandler();
	~BGCompletionPortHandler();

	/** 객체 사용을 시작합니다.*/
	bool Start();

	/** 객체 사용을 종료합니다.*/
	bool Stop();

	/***/
	void OnConnect(BGSession*);

private:
	HANDLE m_hIOCP;
};

