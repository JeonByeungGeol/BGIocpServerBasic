#pragma once

#include "BGSessionEnum.h"
#include "BGIOCompletionHandler.h"

/**
* <pre>
* ���� ������
* </pre>
*/
struct BG_SESSION_DATA
{
	SOCKET m_socket;

	OverlapEx m_recv_overlap;

	int previous_data;

	unsigned char packet[BG_MAX_BUFF_SIZE];

	void Reset() {
		previous_data = 0;
	}
};


/**
* <pre>
* ���� ��ü
* </pre>
*/

class BGSession
{
public:
	~BGSession();

	/** �� ������ ����ϴ� */
	static BGSession* Create(const int& index);

	/** ������ �ʱ�ȭ ��ŵ�ϴ�.*/
	bool Reset();

	/** ���� ���¸� �����մϴ�.*/
	void SetState(ESessionState);

	/** ���� ���¸� Ȯ���մϴ�.*/
	bool IsState(ESessionState);

	/** ������ �ε����� ��ȯ�մϴ�.*/
	int GetIndex();

	/** ������*/
	BG_SESSION_DATA m_Data;

private:
	/** �ܺ� ���� ���� */
	explicit BGSession(const int& index);

	/** ���� �ε��� : ���� �ѹ� ���� �Ǹ�, �ٲ��� �ʽ��ϴ�.*/
	int m_Index;
	
	/** ���� ����*/
	ESessionState m_State;
};

