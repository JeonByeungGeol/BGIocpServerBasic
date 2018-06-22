#pragma once

#include "BGSessionEnum.h"

/**
* <pre>
* 세션 데이터
* </pre>
*/
struct BG_SESSION_DATA
{
	int sampleData1;

	std::string sampleData2;


	void Reset() {
		sampleData1 = 0;
		sampleData2.clear();
	}
};


/**
* <pre>
* 세션 객체
* </pre>
*/

class BGSession
{
public:
	~BGSession();

	/** 새 새션을 만듭니다 */
	static BGSession* Create(const int& index);

	/** 세션을 초기화 시킵니다.*/
	bool Reset();



private:
	/** 외부 생성 금지 */
	explicit BGSession(const int& index);

	/** 세션 인덱스 : 값은 한번 정의 되면, 바뀌지 않습니다.*/
	int m_Index;
	
	/** 세션 상태*/
	ESessionState m_State;

	/** 데이터*/
	BG_SESSION_DATA m_Data;
};

