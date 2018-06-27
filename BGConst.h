#pragma once

/** ���� ���� �̸� */
static const char* const BG_CONFIG_FILE_NAME = "bg_config.ini";

/** ���� Ǯ ũ�� : 10001�� ���� Ǯ ����*/
static const int BG_SESSION_POOL_SIZE = 10001;

/** ���� WorkerThread ��*/
static const int BG_WORKER_THREAD_NUM = 4;

/** IOBuff���� ����� ���� �ִ� ũ��*/
static const int BG_MAX_BUFF_SIZE = 4096;

/** IOCP ��Ŷ�� �����ϱ� ���� ����*/
static const int BG_OP_OFF = 0;
static const int BG_OP_RECV = 1;
static const int BG_OP_SEND = 2;