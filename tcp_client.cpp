// tcp_client.cpp

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>
#define SERVERIP "127.0.0.1"
#define SERVERPORT 9000
#define BUFSIZE 512

// @)))))))))) 데이터 통신에 사용할 변수 글로벌 변수로 뺴둠
int retval;
SOCKET sock;
char buf[BUFSIZE + 1];
int len;

// 소켓 함수 오류 출력 후 종료
void err_quit(const char* msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	MessageBox(NULL, (LPCTSTR)lpMsgBuf, msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
	exit(1);
}

// 소켓 함수 오류 출력
void err_display(const char* msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	printf("[%s] %s", msg, (char*)lpMsgBuf);
	LocalFree(lpMsgBuf);
}

// @)))))))))) 데이터 송신 스레드
DWORD WINAPI SendData(LPVOID arg) {
	while (1) {
		// 데이터 입력
		printf("> ");
		if (fgets(buf, BUFSIZE + 1, stdin) == NULL)
			break;

		// '\n' 문자 제거
		len = strlen(buf);
		if (buf[len - 1] == '\n')
			buf[len - 1] = '\0';

		// 데이터 보내기
		retval = send(sock, buf, strlen(buf), 0);
		if (retval == SOCKET_ERROR) {
			err_display("send()");
			break;
		}
	}
	return 0;
}

// @)))))))))) 데이터 수신 스레드
DWORD WINAPI RecvData(LPVOID arg) {
	while (1) {
		retval = recv(sock, buf, BUFSIZE, 0);
		if (retval == SOCKET_ERROR) {
			err_display("recv()");
			break;
		}

		// 수신 받은 데이터 출력
		buf[retval] = '\0';
		printf("[익명]: %s\n", buf);
	}
	return 0;
}

int main(int argc, char* argv[])
{
	// @)))))))))) 메인화면
	printf("\n준하 채팅에 오신것을 환영합니다!\n");
	printf("\n서버에 접속중입니다.. 잠시만 기다려주세요!\n");
	printf("\n[Source Code] https://github.com/junhaddi/ck-network-chatting.git\n");

	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	// socket()
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET) err_quit("socket()");

	// connect()
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = inet_addr(SERVERIP);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = connect(sock, (SOCKADDR*)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("connect()");

	// @)))))))))) 접속완료 화면
	if (retval == 0) {
		printf("\n접속완료!!\n\n");
	}

	// @)))))))))) 데이터 송신, 수신 스레드 생성
	HANDLE hThread[2];
	hThread[0] = CreateThread(NULL, 0, SendData, NULL, 0, NULL);
	hThread[1] = CreateThread(NULL, 0, RecvData, NULL, 0, NULL);
	WaitForMultipleObjects(2, hThread, TRUE, INFINITE);

	// closesocket()
	closesocket(sock);

	// 윈속 종료
	WSACleanup();
	return 0;
}
