#include "stdafx.h"
#include "BGSession.h"


BGSession::BGSession(const int& index)
	: m_Index(index), m_State(ESessionState::BG_NONE)
{
}


BGSession::~BGSession()
{
}

BGSession * BGSession::Create(const int & index)
{
	return new BGSession(index);
}

bool BGSession::Reset()
{
	m_State = ESessionState::BG_NONE;
	m_Data.Reset();
	
	return true;
}
