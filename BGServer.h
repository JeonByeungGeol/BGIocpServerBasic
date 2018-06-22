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
	/** ���� ������ ���������� �Ǵ��ϱ� ���� ������ ���ؽ�*/
	std::shared_mutex shared_mutex;
	bool m_bServerRunning;

	/** ������ ó���� ���� ������ */
	std::thread acceptThread;

	/** IOCP Worker Thread */
	std::vector<std::thread*> workerThreads;
};

