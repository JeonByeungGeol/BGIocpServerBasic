// BGPrototype.cpp : �ܼ� ���� ���α׷��� ���� �������� �����մϴ�.
//

#include "stdafx.h"


int main()
{
	if (!g_Config.Load()) {
		std::cout << "g_Config.Load Failed!" << std::endl;
		return -1;
	}

	if (!g_LogManager.Start()) {
		std::cout << "g_LogManager.Start Failed!" << std::endl;
		return -1;
	}

	if (!g_SessionManager.Start()) {
		BG_LOG_ERROR("g_SessionManager.Start failed");
		return -1;
	}

	if (!g_Server.Start()) {
		BG_LOG_ERROR("g_Server.Start Failed!");
		return -1;
	}


	while (1)
	{
		char c;
		std::cin >> c;
		if (c == 'e')
		{
			break;
		}
		Sleep(1000);
	}

	if (!g_Server.Stop()) {
		BG_LOG_ERROR("g_Server.Stop Failed!");
		return -1;
	}

	if (!g_SessionManager.Stop()) {
		BG_LOG_ERROR("g_SessionManager.Stop failed");
		return -1;
	}

	if (!g_LogManager.Stop()) {
		std::cout << "g_LogManager.Stop Failed!" << std::endl;
		return -1;
	}

	return 0;
}