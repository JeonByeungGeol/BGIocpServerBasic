#pragma once
class BGServer
{
public:
	BGServer();
	~BGServer();

	/** ������ �����մϴ�.*/
	bool Start();

	/** ������ �����մϴ�.*/
	bool Stop();

	/** ������ �ʱ�ȭ�մϴ�.*/
	bool Initialize();

	/** ������ ���������� Ȯ���մϴ�.*/
	bool IsRunning();

	

private:
	std::shared_mutex shared_mutex;
	bool m_bServerRunning;
};

