#pragma comment (lib, "ws2_32")

#define WIN32_LEAN_AND_MEAN  
#define INITGUID

#include <Windows.h>
#include <WinSock2.h>
#include <thread>
#include <vector>
#include <iostream>
#include <mutex>
#include <set>
#include <chrono>
#include <queue>
#include <mutex>
#include <sqlext.h>
#include <map>

using namespace std;
using namespace std::chrono;

#include "protocol.h"
extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

#define NUM_THREADS	6

#define OP_RECV 1
#define OP_SEND 2
#define OP_MOVE 3

HANDLE g_hIocp;

// 1이면 장애물, 0이면 장애물 없음
bool g_Collsion[BOARD_HEIGHT][BOARD_WIDTH];

// DB 변수 -----------------------
SQLHENV henv;
SQLHDBC hdbc;
SQLHSTMT hstmt;
SQLRETURN retcode;
SQLINTEGER pIndicators[6];
wchar_t user_name[10];
long user_pos_X, user_pos_Y, user_pos_Z;
long user_level, user_exp;
// ----------------------DB 변수 끝

//std::map<wchar_t, bool> id_check_map;

struct event_token {
	int obj_id;
	unsigned int wakeup_time;
	int event_type;
};

class mycomp {
	bool reverse;
public:
	mycomp() {}
	bool operator() (const event_token lhs, const event_token rhs) const
	{
		return (lhs.wakeup_time > rhs.wakeup_time);
	}
};

priority_queue<event_token, vector<event_token>, mycomp> p_queue;
mutex qlock;


struct Player {
	int x, z;
	lua_State* L;
};

struct OverlapEx {
	WSAOVERLAPPED original_overlap;
	int	operation;
	WSABUF recv_buf;
	unsigned char socket_buf[MAX_BUFF_SIZE];
	int packet_size;
};

struct Client {
	int	m_id;
	int target;
	bool is_connected;
	bool is_active;
	SOCKET m_s;
	Player m_player;
	set <int> view_list;
	mutex vl_lock;
	OverlapEx m_recv_overlap;
	int previous_data;
	unsigned char packet[MAX_BUFF_SIZE];
};

Client clients[NUM_OF_NPC];

bool g_isshutdown = false;

void error_display(char *msg, int err_no)
{
	WCHAR *lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, err_no,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	printf("%s", msg);
	wprintf(L"에러%s\n", lpMsgBuf);
	LocalFree(lpMsgBuf);
}

void HandleDiagnosticRecord(SQLHANDLE hHandle,
	SQLSMALLINT hType,
	RETCODE RetCode)
{
	SQLSMALLINT iRec = 0;
	SQLINTEGER  iError;
	WCHAR       wszMessage[1000];
	WCHAR       wszState[SQL_SQLSTATE_SIZE + 1];

	if (RetCode == SQL_INVALID_HANDLE)
	{
		fwprintf(stderr, L"Invalid handle!\n");
		return;
	}

	while (SQLGetDiagRec(hType,
		hHandle,
		++iRec,
		wszState,
		&iError,
		wszMessage,
		(SQLSMALLINT)(sizeof(wszMessage) / sizeof(WCHAR)),
		(SQLSMALLINT *)NULL) == SQL_SUCCESS)
	{
		// Hide data truncated..
		if (wcsncmp(wszState, L"01004", 5))
		{
			fwprintf(stderr, L"[%5.5s] %s (%d)\n", wszState, wszMessage, iError);
		}
	}
}

bool Is_InRange(int i, int j)
{
	int dist = (clients[i].m_player.x - clients[j].m_player.x)
		* (clients[i].m_player.x - clients[j].m_player.x);
	dist += (clients[i].m_player.z - clients[j].m_player.z)
		* (clients[i].m_player.z - clients[j].m_player.z);

	return dist <= VIEW_RADIUS * VIEW_RADIUS;
}

const bool Is_NPC(const int id)
{
	return (id >= NPC_START);
}

void Initialize_NPC()
{
	for (auto i = NPC_START; i < NUM_OF_NPC; ++i)
	{
		clients[i].is_connected = true;
		clients[i].is_active = false;
		clients[i].m_player.x = rand() % BOARD_WIDTH;
		clients[i].m_player.z = rand() % BOARD_HEIGHT;
		lua_State *L = clients[i].m_player.L = luaL_newstate();
		luaL_openlibs(L);
		luaL_loadfile(L, "monster.lua");
		lua_pcall(L, 0, 0, 0);
	}
}

void Initialize_Map()
{
	srand(2011180045);
	for (int i = 0; i < BOARD_HEIGHT; ++i)
		for (int j = 0; j < BOARD_WIDTH; ++j)
			g_Collsion[i][j] = rand() % 10 == 0 ? true : false;
}

void Initialize_DB()
{
	// Allocate environment handle
	retcode = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);

	// Set the ODBC version environment attribute
	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
		retcode = SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (void*)SQL_OV_ODBC3, 0);

		// Allocate connection handle
		if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
			retcode = SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);

			// Set login timeout to 5 seconds
			if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
				SQLSetConnectAttr(hdbc, SQL_LOGIN_TIMEOUT, (SQLPOINTER)5, 0);

				// Connect to data source
				retcode = SQLConnect(hdbc, (SQLWCHAR*)L"sodtms", SQL_NTS,
					(SQLWCHAR*)L"sa", SQL_NTS,
					(SQLWCHAR*)L"gameserver", SQL_NTS);

				// Allocate statement handle
				if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
					retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);

					// Process data
					if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
						cout << "DB Connection Success!" << endl;



					}
					else
						HandleDiagnosticRecord(hdbc, SQL_HANDLE_DBC, retcode);
				}
				else
					SQLDisconnect(hdbc);

			} // 세번째 if문
			else
				SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
		} // 두번째 if문
		else
			SQLFreeHandle(SQL_HANDLE_ENV, henv);
	} // 첫번째 if문
}

const bool Is_Active(const int id)
{
	return clients[id].is_active;
}

void WakeUpNPC(int npc)
{
	clients[npc].is_active = true;
	p_queue.push(event_token{ npc, GetTickCount() + 1000, OP_MOVE });
}

void SleepNPC(int npc)
{
	clients[npc].is_active = false;
}

void SendPacket(int id, unsigned char *packet)
{
	OverlapEx *send_over = new OverlapEx;
	memset(send_over, 0, sizeof(OverlapEx));
	send_over->operation = OP_SEND;
	send_over->recv_buf.buf = reinterpret_cast<CHAR *>(send_over->socket_buf);
	send_over->recv_buf.len = packet[0];
	memcpy(send_over->socket_buf, packet, packet[0]);
	int res = WSASend(clients[id].m_s, &send_over->recv_buf, 1,
		NULL, 0, &send_over->original_overlap, NULL);
	if (0 != res)
	{
		int error_no = WSAGetLastError();
		if (WSA_IO_PENDING != error_no)
			error_display("SendPacket:WSASend", error_no);
		while (true);
	}
}

void SendChatPacket(int id, int dest, wchar_t* mess);

void do_move(int id)
{
	if (false == clients[id].is_active) return;
	set <int> view_list;
	set <int> new_list;

	for (auto j = 0; j < MAX_USER; ++j) {
		if (clients[j].is_connected == false) continue;
		if (Is_InRange(id, j)) view_list.insert(j);
	}

	int x = clients[id].m_player.x;
	int z = clients[id].m_player.z;

	int target = clients[id].target;

	if (clients[target].m_player.x > x) x++;
	else if (clients[target].m_player.x < x) x--;
	else if (clients[target].m_player.z > z) z++;
	else if (clients[target].m_player.z < z) z--;

	// 장애물 처리
	if (!g_Collsion[z][x]) {
		clients[id].m_player.x = x;
		clients[id].m_player.z = z;
	}
	else {
		qlock.lock();
		p_queue.push(event_token{ id, GetTickCount() + 1000,  OP_MOVE });
		qlock.unlock();
		return;
	}

	if (x == clients[target].m_player.x
		&& z == clients[target].m_player.z) {

		SendChatPacket(target, target, L"으악~~~!!!");
	}

	for (auto j = 0; j < MAX_USER; ++j) {
		if (false == clients[j].is_connected) continue;
		if (Is_InRange(id, j)) new_list.insert(j);
	}

	for (auto j : view_list) {
		if (0 != new_list.count(j)) {
			if (Is_NPC(j)) continue;
			sc_packet_pos packet;
			packet.id = id;
			packet.size = sizeof(packet);
			packet.type = SC_POS;
			packet.x = clients[id].m_player.x;
			packet.y = clients[id].m_player.z;
			SendPacket(j, reinterpret_cast<unsigned char *>(&packet));
			//MOVE
		}
		else {
			if (Is_NPC(j)) continue;
			clients[j].vl_lock.lock();
			clients[j].view_list.erase(id);
			clients[j].vl_lock.unlock();
			sc_packet_remove_player packet;
			packet.id = id;
			packet.size = sizeof(packet);
			packet.type = SC_REMOVE_PLAYER;
			SendPacket(j, reinterpret_cast<unsigned char *>(&packet));
			// REMOVE
		}
	}

	bool player_exist = false;
	for (auto j : new_list) {
		if (Is_NPC(j)) continue;
		player_exist = true;
		if (0 != view_list.count(j)) continue; // MOVE
		clients[j].vl_lock.lock();
		clients[j].view_list.insert(id);
		clients[j].vl_lock.unlock();
		sc_packet_put_player packet;  // ADD
		packet.id = id;
		packet.size = sizeof(packet);
		packet.type = SC_PUT_PLAYER;
		packet.x = clients[id].m_player.x;
		packet.y = clients[id].m_player.z;
		SendPacket(j, reinterpret_cast<unsigned char *>(&packet));
	}
	qlock.lock();
	p_queue.push(event_token{ id, GetTickCount() + 1000,  OP_MOVE });
	qlock.unlock();

	if (false == player_exist) SleepNPC(id);
}

void Initialize_Server()
{
	WSADATA	wsadata;
	_wsetlocale(LC_ALL, L"korean");
	WSAStartup(MAKEWORD(2, 2), &wsadata);
	g_hIocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0);
	for (auto i = 0; i < MAX_USER; ++i) {
		clients[i].m_recv_overlap.recv_buf.buf =
			reinterpret_cast<CHAR *>(clients[i].m_recv_overlap.socket_buf);
		clients[i].m_recv_overlap.recv_buf.len = MAX_BUFF_SIZE;
		clients[i].m_recv_overlap.operation = OP_RECV;
		clients[i].is_connected = false;
	}
	Initialize_NPC();

	Initialize_Map();

	Initialize_DB();
}

void SendPutPlayerPacket(int client, int player)
{
	sc_packet_put_player packet;
	packet.id = player;
	packet.size = sizeof(packet);
	packet.type = SC_PUT_PLAYER;
	packet.x = clients[player].m_player.x;
	packet.y = clients[player].m_player.z;
	SendPacket(client, reinterpret_cast<unsigned char *>(&packet));
}

void SendRemovePlayerPacket(int client, int player)
{
	sc_packet_remove_player packet;
	packet.id = player;
	packet.size = sizeof(packet);
	packet.type = SC_REMOVE_PLAYER;
	SendPacket(client, reinterpret_cast<unsigned char *>(&packet));
}

void Success_Login(int new_id)
{
	ZeroMemory(&clients[new_id].m_recv_overlap.original_overlap,
		sizeof(clients[new_id].m_recv_overlap.original_overlap));

	sc_packet_put_player put_player_packet;
	put_player_packet.id = new_id;
	put_player_packet.size = sizeof(put_player_packet);
	put_player_packet.type = SC_PUT_PLAYER;
	put_player_packet.x = clients[new_id].m_player.x;
	put_player_packet.y = clients[new_id].m_player.z;

	clients[new_id].vl_lock.lock();
	clients[new_id].view_list.clear();
	clients[new_id].vl_lock.unlock();

	SendPacket(new_id,
		reinterpret_cast<unsigned char*>(&put_player_packet));



	// 첫번째 루프와 두번째 루프가 중간에 i의 view_list가 바뀔수도 있지만, 상관없음 -> 생각해보자
	for (auto i = 0; i < MAX_USER; ++i)
	{
		if (true == clients[i].is_connected)
		{
			if (false == Is_InRange(new_id, i)) continue;
			clients[i].vl_lock.lock();
			if (new_id != i) /*continue;*/ clients[i].view_list.insert(new_id);
			clients[i].vl_lock.unlock();
			SendPacket(i, reinterpret_cast<unsigned char*>(&put_player_packet));
		}
	}
	clients[new_id].is_connected = true;

	for (auto i = 0; i < NUM_OF_NPC; ++i) {
		if (false == clients[i].is_connected) continue;
		if (i == new_id) continue;
		if (false == Is_InRange(new_id, i)) continue;
		clients[new_id].vl_lock.lock();
		clients[new_id].view_list.insert(i);
		clients[new_id].vl_lock.unlock();
		put_player_packet.id = i;
		put_player_packet.x = clients[i].m_player.x;
		put_player_packet.y = clients[i].m_player.z;
		SendPacket(new_id, reinterpret_cast<unsigned char*>(&put_player_packet));
	}
}

void SendChatPacket(int id, int dest, wchar_t* mess)
{
	sc_packet_chat chat_test;
	chat_test.id = id;
	wcscpy(chat_test.message, mess);
	chat_test.size = sizeof(sc_packet_chat);
	chat_test.type = SC_CHAT;

	SendPacket(dest, reinterpret_cast<unsigned char*>(&chat_test));
}

void ProcessPacket(int id, unsigned char *packet)
{
	int dx = clients[id].m_player.x;
	int dz = clients[id].m_player.z;
	unsigned char packet_type = packet[1];

	switch (packet_type) {
	case CS_MAP: {
		sc_packet_map packet_map;
		packet_map.size = sizeof(sc_packet_map);
		packet_map.type = SC_MAP;
		packet_map.seed = 2011180045;
		SendPacket(id, reinterpret_cast<unsigned char*>(&packet_map));
		return;
	}
	case CS_LOGIN: {
		cs_packet_login* packet_login = reinterpret_cast<cs_packet_login*>(packet);
		char strid[30];
		char strpassword[30];
		wcstombs(strid, packet_login->id, 30);
		wcstombs(strpassword, packet_login->password, 30);
		cout << strid << "\t" << strpassword << endl;

		wchar_t strSQL[100];
		char c_strSQL[100] = "EXEC [game_server].[dbo].[check_id] ";
		strcat(c_strSQL, strid);

		mbstowcs(strSQL, c_strSQL, 100);

		//쿼리문
		retcode = SQLExecDirect(hstmt, (SQLWCHAR*)strSQL, SQL_NTS);
		if (retcode == SQL_ERROR)
		{
			//error handle
			HandleDiagnosticRecord(hdbc, SQL_HANDLE_DBC, retcode);
		}
		else
		{
			//데이터 읽기
			SQLBindCol(hstmt, 1, SQL_C_WCHAR, (SQLPOINTER)user_name, 22, &pIndicators[0]);
			SQLBindCol(hstmt, 2, SQL_C_LONG, (SQLPOINTER)&user_level, sizeof(user_level), &pIndicators[1]);
			SQLBindCol(hstmt, 3, SQL_C_LONG, (SQLPOINTER)&user_exp, sizeof(user_exp), &pIndicators[2]);
			SQLBindCol(hstmt, 4, SQL_C_LONG, (SQLPOINTER)&user_pos_X, sizeof(user_pos_X), &pIndicators[3]);
			SQLBindCol(hstmt, 5, SQL_C_LONG, (SQLPOINTER)&user_pos_Y, sizeof(user_pos_Y), &pIndicators[4]);
			SQLBindCol(hstmt, 6, SQL_C_LONG, (SQLPOINTER)&user_pos_Z, sizeof(user_pos_Z), &pIndicators[5]);

			if (SQLFetch(hstmt) == SQL_SUCCESS)
			{
				wcout << L"name - " << user_name << endl;
				cout << "level : " << user_level << "\t exp : " << user_exp << endl;
				cout << "x : " << user_pos_X << "\ty : " << user_pos_Y << "\tz : " << user_pos_Z << endl << endl;

				sc_packet_login_ok packet_login_ok;
				packet_login_ok.size = sizeof(sc_packet_login_ok);
				packet_login_ok.type = SC_LOGIN_OK;
				packet_login_ok.level = user_level;
				packet_login_ok.exp = user_exp;
				packet_login_ok.x = (int)user_pos_X;
				packet_login_ok.y = (int)user_pos_Z;

				clients[id].m_player.x = (int)user_pos_X;
				clients[id].m_player.z = (int)user_pos_Z;

				SendPacket(id, reinterpret_cast<unsigned char*>(&packet_login_ok));

				Success_Login(id);
			}
			else {
				sc_packet_login_fail packet_login_fail;
				packet_login_fail.size = sizeof(sc_packet_login_fail);
				packet_login_fail.type = SC_LOGIN_FAIL;

				SendPacket(id, reinterpret_cast<unsigned char*>(&packet_login_fail));
			}
		}	// 쿼리문 종료

		SQLCloseCursor(hstmt);
		return;
	}
	case CS_UP:		dz--; break;
	case CS_DOWN:	dz++; break;
	case CS_LEFT:	dx--; break;
	case CS_RIGHT:	dx++; break;
	default: cout << "Unknown Packet Type Detected!!\n";
		exit(-1);
	}
	if (dz < 0) dz = 0;
	if (dz >= BOARD_HEIGHT) dz = BOARD_HEIGHT - 1;
	if (dx < 0) dx = 0;
	if (dx >= BOARD_WIDTH) dx = BOARD_WIDTH - 1;


	set<int> arrounds;
	for (int i = 0; i < MAX_USER; i++) {
		if (false == clients[i].is_connected) continue;
		if (false == Is_InRange(id, i)) continue;
		arrounds.insert(i);
	}

	if (g_Collsion[dz][dx]) {
		for (int i : arrounds)
			SendChatPacket(id, i, L"쿵!");
	}

	if (g_Collsion[dz][dx]) return;

	clients[id].m_player.x = dx;
	clients[id].m_player.z = dz;

	sc_packet_pos pos_packet;
	pos_packet.id = id;
	pos_packet.size = sizeof(sc_packet_pos);
	pos_packet.type = SC_POS;
	pos_packet.x = dx;
	pos_packet.y = dz;

	SendPacket(id, reinterpret_cast<unsigned char*>(&pos_packet));

	set<int> new_list;
	for (int i = 0; i < NUM_OF_NPC; i++) {
		if (false == clients[i].is_connected) continue;
		if (false == Is_InRange(id, i)) continue;
		if (i == id) continue;
		new_list.insert(i);
	}

	for (auto npc : new_list) {
		if (!Is_NPC(npc)) continue;
		if (Is_Active(npc)) continue;
		WakeUpNPC(npc);
		clients[npc].target = id;
	}

	// 새로 시야에 들어온 객체 처리
	for (auto i : new_list)
	{
		clients[id].vl_lock.lock();
		bool new_one(0 == clients[id].view_list.count(i));
		if (true == new_one) clients[id].view_list.insert(i);
		clients[id].vl_lock.unlock();
		if (true == new_one) SendPutPlayerPacket(id, i);

		if (Is_NPC(i)) continue;
		clients[i].vl_lock.lock();
		if (0 == clients[i].view_list.count(id)) {
			clients[i].view_list.insert(id);
			clients[i].vl_lock.unlock();
			SendPutPlayerPacket(i, id);
		}
		else {
			clients[i].vl_lock.unlock();
			SendPacket(i, reinterpret_cast<unsigned char *>(&pos_packet));
		}
	}
	// 시야에서 유지되는 객체 처리 => 위에서 같이 처리

	// 시야에서 벗어난 객체 처리
	vector <int> remove_list;
	clients[id].vl_lock.lock();
	for (int i : clients[id].view_list) {
		if (0 != new_list.count(i)) continue;
		//	clients[id].view_list.erase(i);
		remove_list.push_back(i);
	}
	for (auto i : remove_list) clients[id].view_list.erase(i);
	clients[id].vl_lock.unlock();

	for (auto i : remove_list) {
		SendRemovePlayerPacket(id, i);
		if (Is_NPC(i)) continue;
		clients[i].vl_lock.lock();
		if (0 == clients[i].view_list.count(id)) {
			clients[i].vl_lock.unlock();
			continue;
		}
		clients[i].view_list.erase(id);
		clients[i].vl_lock.unlock();
		SendRemovePlayerPacket(i, id);
	}
}

void worker_thread()
{
	DWORD io_size, key;
	OverlapEx *overlap;
	BOOL result;

	while (true)
	{
		result = GetQueuedCompletionStatus(g_hIocp, &io_size, &key,
			reinterpret_cast<LPOVERLAPPED *>(&overlap), INFINITE);
		if (FALSE == result) {
			// Error를 처리한다.
		}
		if (0 == io_size) {
			closesocket(clients[key].m_s);

			sc_packet_remove_player rp_packet;
			rp_packet.id = key;
			rp_packet.size = sizeof(sc_packet_remove_player);
			rp_packet.type = SC_REMOVE_PLAYER;
			for (auto i = 0; i < MAX_USER; ++i) {
				if (false == clients[i].is_connected) continue;
				if (key == i) continue;

				clients[i].vl_lock.lock();
				if (0 != clients[i].view_list.count(key)) {
					clients[i].view_list.erase(key);
					clients[i].vl_lock.unlock();
					SendPacket(i,
						reinterpret_cast<unsigned char *>(&rp_packet));
				}
				else clients[i].vl_lock.unlock();
			}
			clients[key].is_connected = false;
			continue;
		}
		if (OP_RECV == overlap->operation) {
			unsigned char *buf_ptr = overlap->socket_buf;
			int remained = io_size;

			while (0 < remained) {
				if (0 == clients[key].m_recv_overlap.packet_size) {
					clients[key].m_recv_overlap.packet_size = buf_ptr[0];
				}
				int required = clients[key].m_recv_overlap.packet_size
					- clients[key].previous_data;
				if (remained >= required) {
					memcpy(clients[key].packet + clients[key].previous_data,
						buf_ptr,
						required);
					ProcessPacket(key, clients[key].packet);
					remained -= required;
					buf_ptr += required;
					clients[key].m_recv_overlap.packet_size = 0;
					clients[key].previous_data = 0;
				}
				else {
					memcpy(clients[key].packet + clients[key].previous_data,
						buf_ptr, remained);
					clients[key].previous_data += remained;
					remained = 0;
					buf_ptr += remained;
				}
			}
			DWORD flags = 0;
			WSARecv(clients[key].m_s,
				&clients[key].m_recv_overlap.recv_buf,
				1, NULL, &flags,
				reinterpret_cast<LPWSAOVERLAPPED>(&clients[key].m_recv_overlap),
				NULL);
		}
		else if (OP_SEND == overlap->operation) {
			// ioSize하고 실제 보낸 크기 비교 후 소켓 접속 끊기
			delete overlap;
		}
		else if (OP_MOVE == overlap->operation) {
			do_move(key);
			static int count = 0;
			count++;
			if (0 == (count % 100000))
				cout << "100000 NPC moved\n";
			delete overlap;
		}
		else {
			cout << "Unknown Event on worker_thread\n";
			while (true);
		}
	}
}

void accept_thread()
{
	struct sockaddr_in listen_addr;

	SOCKET accept_socket = WSASocket(AF_INET, SOCK_STREAM,
		IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);

	ZeroMemory(&listen_addr, sizeof(listen_addr));
	listen_addr.sin_family = AF_INET;
	listen_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	listen_addr.sin_port = htons(MY_SERVER_PORT);
	ZeroMemory(&listen_addr.sin_zero, 8);
	::bind(accept_socket,
		reinterpret_cast<sockaddr *>(&listen_addr),
		sizeof(listen_addr));

	listen(accept_socket, 10);

	while (true) {
		struct sockaddr_in client_addr;
		int addr_size = sizeof(client_addr);

		SOCKET new_client = WSAAccept(accept_socket,
			reinterpret_cast<sockaddr *>(&client_addr),
			&addr_size, NULL, NULL);


		int new_id = -1;
		for (auto i = 0; i < MAX_USER; ++i) {
			if (clients[i].is_connected == false) {
				new_id = i;
				break;
			}
		}

		if (-1 == new_id) {
			cout << "Maximum User Number Fail\n";
			closesocket(new_client);
			continue;
		}

		CreateIoCompletionPort(reinterpret_cast<HANDLE>(new_client),
			g_hIocp, new_id, 0);

		clients[new_id].m_s = new_client;
		clients[new_id].m_id = new_id;
		clients[new_id].m_player.x = 4;
		clients[new_id].m_player.z = 4;
		clients[new_id].m_recv_overlap.operation = OP_RECV;
		clients[new_id].m_recv_overlap.packet_size = 0;
		clients[new_id].previous_data = 0;



		DWORD flags = 0;
		int res = WSARecv(new_client,
			&clients[new_id].m_recv_overlap.recv_buf, 1, NULL,
			&flags,
			&clients[new_id].m_recv_overlap.original_overlap, NULL);
		if (0 != res) {
			int error_no = WSAGetLastError();
			if (WSA_IO_PENDING != error_no) {
				error_display("AcceptThread:WSARecv", error_no);
			}
		}
	}
}

void do_ai_move() {
	for (auto i = NPC_START; i < NUM_OF_NPC; ++i)
	{
		set <int> view_list;
		set <int> new_list;

		for (auto j = 0; j < MAX_USER; ++j) {
			if (clients[j].is_connected == false) continue;
			if (Is_InRange(i, j)) view_list.insert(j);
		}

		switch (rand() % 4) {
		case 0: if (clients[i].m_player.x < BOARD_WIDTH - 1)
			clients[i].m_player.x++; break;
		case 1: if (clients[i].m_player.x > 0)
			clients[i].m_player.x--; break;
		case 2: if (clients[i].m_player.z < BOARD_HEIGHT - 1)
			clients[i].m_player.z++; break;
		case 3: if (clients[i].m_player.z > 0)
			clients[i].m_player.z--; break;
		}
		for (auto j = 0; j < MAX_USER; ++j) {
			if (false == clients[j].is_connected) continue;
			if (Is_InRange(i, j)) new_list.insert(j);
		}

		for (auto j : view_list) {
			if (0 != new_list.count(j)) {
				if (Is_NPC(j)) continue;
				sc_packet_pos packet;
				packet.id = i;
				packet.size = sizeof(packet);
				packet.type = SC_POS;
				packet.x = clients[i].m_player.x;
				packet.y = clients[i].m_player.z;
				SendPacket(j, reinterpret_cast<unsigned char *>(&packet));
				//MOVE
			}
			else {
				if (Is_NPC(j)) continue;
				clients[j].vl_lock.lock();
				clients[j].view_list.erase(i);
				clients[j].vl_lock.unlock();
				sc_packet_remove_player packet;
				packet.id = i;
				packet.size = sizeof(packet);
				packet.type = SC_REMOVE_PLAYER;
				SendPacket(j, reinterpret_cast<unsigned char *>(&packet));
				// REMOVE
			}
		}
		for (auto j : new_list) {
			if (Is_NPC(j)) continue;
			if (0 != view_list.count(j)) continue; // MOVE
			clients[j].vl_lock.lock();
			clients[j].view_list.insert(i);
			clients[j].vl_lock.unlock();
			sc_packet_put_player packet;  // ADD
			packet.id = i;
			packet.size = sizeof(packet);
			packet.type = SC_PUT_PLAYER;
			packet.x = clients[i].m_player.x;
			packet.y = clients[i].m_player.z;
			SendPacket(j, reinterpret_cast<unsigned char *>(&packet));
		}
	}
}

void heart_beat(int id)
{
	set <int> view_list;
	set <int> new_list;

	// if (now - last_move_time < 10000) continue;

	for (auto j = 0; j < MAX_USER; ++j) {
		if (clients[j].is_connected == false) continue;
		if (Is_InRange(id, j)) view_list.insert(j);
	}

	switch (rand() % 4) {
	case 0: if (clients[id].m_player.x < BOARD_WIDTH - 1)
		clients[id].m_player.x++; break;
	case 1: if (clients[id].m_player.x > 0)
		clients[id].m_player.x--; break;
	case 2: if (clients[id].m_player.z < BOARD_HEIGHT - 1)
		clients[id].m_player.z++; break;
	case 3: if (clients[id].m_player.z > 0)
		clients[id].m_player.z--; break;
	}
	for (auto j = 0; j < MAX_USER; ++j) {
		if (false == clients[j].is_connected) continue;
		if (Is_InRange(id, j)) new_list.insert(j);
	}

	for (auto j : view_list) {
		if (0 != new_list.count(j)) {
			if (Is_NPC(j)) continue;
			sc_packet_pos packet;
			packet.id = id;
			packet.size = sizeof(packet);
			packet.type = SC_POS;
			packet.x = clients[id].m_player.x;
			packet.y = clients[id].m_player.z;
			SendPacket(j, reinterpret_cast<unsigned char *>(&packet));
			//MOVE
		}
		else {
			if (Is_NPC(j)) continue;
			clients[j].vl_lock.lock();
			clients[j].view_list.erase(id);
			clients[j].vl_lock.unlock();
			sc_packet_remove_player packet;
			packet.id = id;
			packet.size = sizeof(packet);
			packet.type = SC_REMOVE_PLAYER;
			SendPacket(j, reinterpret_cast<unsigned char *>(&packet));
			// REMOVE
		}
	}
	for (auto j : new_list) {
		if (Is_NPC(j)) continue;
		if (0 != view_list.count(j)) continue; // MOVE
		clients[j].vl_lock.lock();
		clients[j].view_list.insert(id);
		clients[j].vl_lock.unlock();
		sc_packet_put_player packet;  // ADD
		packet.id = id;
		packet.size = sizeof(packet);
		packet.type = SC_PUT_PLAYER;
		packet.x = clients[id].m_player.x;
		packet.y = clients[id].m_player.z;
		SendPacket(j, reinterpret_cast<unsigned char *>(&packet));
	}
}

void ai_thread()
{
	while (true) {
		auto start = high_resolution_clock::now();
		for (auto i = NPC_START; i < NUM_OF_NPC; i++)
			heart_beat(i);
		auto du = high_resolution_clock::now() - start;
		cout << "AI time : " << duration_cast<milliseconds>(du).count();
		cout << endl;
		Sleep(1000);
	}
}

void timer_thread()
{
	while (true)
	{
		Sleep(1);
		qlock.lock();
		while (false == p_queue.empty())
		{
			if (p_queue.top().wakeup_time > GetTickCount()) break;
			event_token ev = p_queue.top();
			p_queue.pop();
			qlock.unlock();
			OverlapEx *send_over = new OverlapEx;
			memset(send_over, 0, sizeof(OverlapEx));
			send_over->operation = OP_MOVE;
			PostQueuedCompletionStatus(g_hIocp, 1, ev.obj_id, &(send_over->original_overlap));
			qlock.lock();
		}
		qlock.unlock();
	}
}

int main()
{
	vector <thread *> worker_threads;
	thread accept_threads;

	Initialize_Server();
	for (auto i = 0; i < NUM_THREADS; ++i)
		worker_threads.push_back(new thread{ worker_thread });
	accept_threads = thread{ accept_thread };
	//	thread ai_threads = thread{ ai_thread };
	thread timer_threads = thread{ timer_thread };
	while (g_isshutdown == false)
	{
		Sleep(1000);
	}
	for (auto th : worker_threads) {
		th->join();
		delete th;
	}
	accept_threads.join();
	WSACleanup();
}