#include "stdafx.h"
#include "Protocol.h"

// ���� �Լ� ���� ��� �� ����
void err_quit(char *msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	MessageBox(NULL, (LPCTSTR)lpMsgBuf, (LPCWSTR)msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
	exit(1);
}

// ���� �Լ� ���� ���
void err_display(char *msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	printf("[%s] %s", msg, (char *)lpMsgBuf);
	LocalFree(lpMsgBuf);
}


void recvThreadFunc(SOCKET* sock) {
	char recvData[MAX_PACKET_SIZE] = { 0 };
	int retval;
	// ������ �ޱ�
	while (1) {
		ZeroMemory(recvData, sizeof(recvData));
		retval = recv(*sock, recvData, sizeof(STOC_SYNC), 0);
		if (retval == SOCKET_ERROR) {
			err_display("recv()");
			break;
		}
		else if (retval == 0)
			break;
		HEADER* pHeader = (HEADER*)recvData;
		STOC_SYNC *outputBuf = (STOC_SYNC*)((UCHAR*)recvData + sizeof(HEADER));
		switch (pHeader->byPacketID) {
		case PAK_ID:
			printf("�����߽��ϴ�. �ο����� ���̵�� [ %d ] �Դϴ�.", outputBuf->ID);
			break;
		case PAK_REG:
			printf("\n[ %d ] �� Ŭ�� �����߽��ϴ�.", outputBuf->ID);
			printf("\n[���� ������]:");
		case PAK_SYNC:
			// ���� ������ ���
			printf("\n[%d]�� Ŭ�� �Է��� ����: %s", outputBuf->ID, outputBuf->data);
			printf("\n[���� ������]:");
			break;
		}
		

		
	}
}

int main(int argc, char *argv[])
{
	int retval;

	// ���� �ʱ�ȭ
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	// socket()
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET) err_quit("socket()");

	// connect()
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = inet_addr(SERVERIP);
	serveraddr.sin_port = htons(SERVER_PORT);
	retval = connect(sock, (SOCKADDR *)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("connect()");

	// ������ ��ſ� ����� ����
	char sendData[MAX_PACKET_SIZE] = { 0 };
	char inputBuf[MAX_PACKET_SIZE - 2];
	int len;

	// ���ú꽺����
	std::thread recvThread { recvThreadFunc , &sock};

	//�α���
	HEADER *phead = (HEADER*)sendData;
	phead->byPacketID = PAK_ID;
	phead->ucSize = sizeof(HEADER);
	retval = send(sock, sendData, phead->ucSize, 0);

	// ������ ������ ���
	while (1) {
		if (fgets(inputBuf, MAX_PACKET_SIZE - 2, stdin) == NULL)
			break;
		
		// '\n' ���� ����
		len = strlen(inputBuf);
		if (inputBuf[len - 1] == '\n')
			inputBuf[len - 1] = '\0';
		if (strlen(inputBuf) == 0)
			continue;

		// ����� ������ ���̱�
		HEADER *phead = (HEADER*)sendData;
		phead->byPacketID = PAK_SYNC;
		phead->ucSize = strlen(inputBuf)+1 + sizeof(HEADER);

		CTOS_SYNC *pData = (CTOS_SYNC*)(sendData + sizeof(HEADER));
		memcpy(pData->data, inputBuf, strlen(inputBuf) + 1);

		// ������ ������
		retval = send(sock, sendData, phead->ucSize, 0);
		if (retval == SOCKET_ERROR) {
			err_display("send()");
			break;
		}
	}

	// closesocket()
	closesocket(sock);

	// ���� ����
	WSACleanup();
	return 0;
}