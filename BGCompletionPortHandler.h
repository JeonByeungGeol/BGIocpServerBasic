#pragma once
class BGCompletionPortHandler
{
public:
	BGCompletionPortHandler();
	~BGCompletionPortHandler();

	/** ��ü ����� �����մϴ�.*/
	bool Start();

	/** ��ü ����� �����մϴ�.*/
	bool Stop();

	/***/
	void OnConnect(BGSession*);

private:
	HANDLE m_hIOCP;
};

