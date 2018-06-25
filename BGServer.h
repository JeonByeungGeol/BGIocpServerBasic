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
		
	/** Ŭ���̾�Ʈ�κ��� ������ ó���ϴ� �۾��� �����մϴ�.*/
	bool ListenStart();

	/** Ŭ���̾�Ʈ�κ��� ���ӿ�û�� ���� �� �ְ� �ϴ� �Լ�*/
	void Accept();

	/** Ŭ���̾�Ʈ�� ���ӿ�û�� ó���ϴ� �۾��Դϴ�.*/
	bool AcceptProcess(SOCKET&);

	/** ��Ŷ�� �����ؼ� ó���ϴ� �������Լ��Դϴ�.*/
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

	/** Ŭ���̾�Ʈ ������ ���� ����*/
	SOCKET m_ListenSocket;
};

