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

	/** IOCP �ڵ��� ����ϴ�.*/
	HANDLE GetIOCP();

	/** IOCP �ڵ鿡 �����մϴ�.*/
	void OnConnect(BGSession*);

private:
	HANDLE m_hIOCP;
};

