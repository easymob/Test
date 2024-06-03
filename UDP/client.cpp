#include <iostream>
//#include <strings.h>
#include <string.h>
#include <sys/types.h>     
#if defined(__WIN32__) || defined(_WIN32)
#include <winsock2.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif
#include <stdio.h>
#include <stdlib.h>
using namespace std;
 
int main()
{
#if defined(__WIN32__) || defined(_WIN32)
	WSADATA wsa = { 0 };
	WSAStartup(MAKEWORD(2, 2), &wsa);
#endif
	int sockfd;
	struct sockaddr_in des_addr;
	int r;
	char sendline[1024] = {};
	const char on = 1;
	
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on)); //设置套接字选项
	ZeroMemory(&des_addr, sizeof(des_addr));
	des_addr.sin_family = AF_INET;
	des_addr.sin_addr.s_addr = inet_addr("192.168.1.255"); //广播地址
	des_addr.sin_port = htons(9999);
	r = sendto(sockfd, sendline, 1024, 0, (struct sockaddr*)&des_addr, sizeof(des_addr));
	if (r <= 0)
	{
		perror("");
		exit(-1);
	}
//	close(sockfd);
	cout << "finish" << endl;
#if defined(__WIN32__) || defined(_WIN32)
	WSACleanup();
#endif
	return 0;
}
