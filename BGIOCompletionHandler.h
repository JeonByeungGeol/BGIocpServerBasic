#pragma once
class BGIOCompletionHandler
{
public:
	BGIOCompletionHandler();
	~BGIOCompletionHandler();

	/** ��ü ����� �����մϴ�.*/
	bool Start();

	/** ��ü ����� �����մϴ�.*/
	bool Stop();

	/***/
	void OnConnect(BGSession*);

private:
	HANDLE m_hIOCP;
};

