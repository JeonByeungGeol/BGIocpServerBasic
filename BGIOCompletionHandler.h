#pragma once

struct OverlapEx {
	WSAOVERLAPPED original_overlap;
	int	operation;
	WSABUF recv_buf;
	unsigned char socket_buf[BG_MAX_BUFF_SIZE];
	int packet_size;
};

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

