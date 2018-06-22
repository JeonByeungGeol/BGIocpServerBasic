#pragma once

#include "BGSession.h"

/**
* <pre>
* 세션을 관리
* </pre>
*/

class BGSessionManager
{
	using UnusedSessionQueue = std::queue<BGSession*>;
	
public:
	BGSessionManager();
	~BGSessionManager();

	/** 객체 사용을 시작합니다.*/
	bool Start();

	/** 객체 사용을 종료합니다.*/
	bool Stop();
	
	/** 세션을 얻어옵니다.*/
	BGSession* GetSession(int index);

	/** 새로 사용할 세션을 얻어옵니다.*/
	BGSession* GetSessionNew();

	/** 사용하지 않는 세션을 반납합니다.*/
	bool ReturnSession(BGSession*);
	
	/** 사용할 세션을 등록합니다.*/
	bool AddSession(BGSession*);

	/** 사용하지 않는 세션을 반납합니다.*/
	bool RemoveSession(BGSession*);
	
private:
	/** 세션 풀 */
	std::array<BGSession*, BG_SESSION_POOL_SIZE> m_SessionArray;

	/** 사용할 수 있는 세션 큐 */
	UnusedSessionQueue m_UnusedSessionQueue;

	/** UnusedSessionQueue를 동기화 시키는 뮤텍스*/
	std::mutex m_SessionQueueMutex;
	
};

