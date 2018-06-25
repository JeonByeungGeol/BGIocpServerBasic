#pragma once
#pragma comment (lib, "ws2_32")

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
		
	/** ������ Accpet ó���ϴ� �Լ�*/
	void Accept();

	/** */
	void Run();

private:
	/** Accept�Լ��� �����忡�� �����ϱ� ���� �Լ�*/
	std::thread* AcceptSpawn();

	/** Run�Լ��� �����忡�� �����ϱ� ���� �Լ�*/
	std::thread* RunSpawn();

private:
	/** ���� ������ ���������� �Ǵ��ϱ� ���� ������ ���ؽ�*/
	std::shared_mutex shared_mutex;
	bool m_bServerRunning;

	/** ������ ó���� ���� ������ */
	std::thread* acceptThread;

	/** IOCP Worker Thread */
	std::vector<std::thread*> workerThreads;

	
};

