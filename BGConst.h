#pragma once

/** 설정 파일 이름 */
static const char* const BG_CONFIG_FILE_NAME = "bg_config.ini";

/** 세션 풀 크기 : 10001개 세션 풀 유지*/
static const int BG_SESSION_POOL_SIZE = 10001;

/** 서버 WorkerThread 수*/
static const int BG_WORKER_THREAD_NUM = 4;

/** IOBuff에서 사용할 버퍼 최대 크기*/
static const int BG_MAX_BUFF_SIZE = 4096;

/** IOCP 패킷을 구분하기 위한 정의*/
static const int BG_OP_RECV = 1;
static const int BG_OP_SEND = 2;