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

void BGSession::SetState(ESessionState state)
{
	m_State = state;
}

bool BGSession::IsState(ESessionState state)
{
	return (m_State == state);
}

int BGSession::GetIndex()
{
	return m_Index;
}