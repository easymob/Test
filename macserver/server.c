#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#if defined(__WIN32__) || defined(_WIN32)
#include <winsock2.h>
#else
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#endif

#define MAC_REQUEST "new mac address request"
#define MACFILENAME "macaddress.txt"
#define MACSERVERLOG "macserver.log"
#define LENGTH  	512             	// Buffer length
#define MACLEN		17
#define PORT 5010

int FileGetLine(FILE *fp, char *buffer, int maxlen)
{
	int i, j;
	char ch1;

	for (i = 0, j = 0; i < maxlen; j++)
	{
		if (fread(&ch1, sizeof(char), 1, fp) != 1)
		{
			if (feof(fp) != 0)
			{
				if (j == 0)
					return -1; /* 文件结束 */
				else
					break;
			}
			if (ferror(fp) != 0)
				return -2; /* 读文件出错 */
			return -2;
		}
		else
		{
			if (ch1 == '\n' || ch1 == 0x00)
				break; /* 换行 */
			if (ch1 == '\f' || ch1 == 0x1A) /* '\f':换页符也算有效字符 */
			{
				buffer[i++] = ch1;
				break;
			}
			if (ch1 != '\r')
				buffer[i++] = ch1; /* 忽略回车符 */
		}
	}
	buffer[i] = '\0';
	return i;
}

int getmacaddress(char* buffer, int len)
{
	int ret = -1;
	FILE *fp;
	int b1, b2, b3, b4, b5, b6;
	int mac;
	if ((fp = fopen(MACFILENAME, "r")) != NULL)
	{
		if (FileGetLine(fp, buffer, len) == len)
		{
			fclose(fp);
			//printf("read mac : %s\n", buffer);
			sscanf(buffer, "%x:%x:%x:%x:%x:%x", &b1, &b2, &b3, &b4, &b5, &b6);
			mac = (b3 << 24) | (b4 << 16) | (b5 << 8) | b6;
			if (mac == (-1))
			{
				printf("No MAC address available!\n");
			}
			else
			{
				mac += 2;
				b3 = (mac & 0xFF000000) >> 24;
				b4 = (mac & 0x00FF0000) >> 16;
				b5 = (mac & 0x0000FF00) >> 8;
				b6 = mac & 0xFF;
				sprintf(buffer, "%02x:%02x:%02x:%02x:%02x:%02x", b1, b2, b3, b4, b5, b6);
				if ((fp = fopen(MACFILENAME, "w")) != NULL)
				{
					fputs(buffer, fp);
					fclose(fp);
					ret = 0;
				}
			}
		}
		else
		{
			printf("read file %s error\n", MACFILENAME);
			fclose(fp);
		}
	}
	else
		printf("open file %s error\n", MACFILENAME);
	return ret;
}

int main()
{
#if defined(__WIN32__) || defined(_WIN32)
	WSADATA wsa = { 0 };
	WSAStartup(MAKEWORD(2, 2), &wsa);
#endif
	int sockfd;                        // Socket file descriptor
	int num;
	int sin_size;                      	// to store struct size
	char revbuf[LENGTH];          	// Send buffer
	char macbuf[MACLEN+1];
	char strbuf[1024];
	struct sockaddr_in addr_local;
	struct sockaddr_in addr_remote;
	FILE *fp;
	/* Get the Socket file descriptor */
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
	{
		printf("ERROR: Failed to obtain Socket Despcritor.\n");
		return (0);
	}

	/* Fill the local socket address struct */
	addr_local.sin_family = AF_INET;           		// Protocol Family
	addr_local.sin_port = htons(PORT);         		// Port number
	addr_local.sin_addr.s_addr = htonl(INADDR_ANY);  // AutoFill local address
	memset(addr_local.sin_zero, 0, 8);          	// Flush the rest of struct

	/*  Blind a special Port */
	if (bind(sockfd, (struct sockaddr*) &addr_local, sizeof(struct sockaddr))
			== -1)
	{
		printf("ERROR: Failed to bind Port %d.\n", PORT);
		return (0);
	}
	else
	{
		printf("OK: Bind the Port %d sucessfully.\n", PORT);
	}

	while(1)
	{
		sin_size = sizeof(struct sockaddr);
		if ((num = recvfrom(sockfd, revbuf, LENGTH, 0, (struct sockaddr *) &addr_remote, &sin_size)) == -1)
		{
			printf("ERROR!\n");
		}
		else
		{
			revbuf[num] = 0;
			if (strcmp(revbuf, MAC_REQUEST) == 0)
			{
				if (getmacaddress(macbuf, MACLEN) == 0)
				{
					num = sendto(sockfd, macbuf, strlen(macbuf), 0, (struct sockaddr *)&addr_remote, sin_size);
					if (num >= 0)
					{
						time_t t = time(0);
						char buffer[20] = {0};
						strftime(buffer, 20, "%Y-%m-%d %H:%M:%S", localtime(&t));
						sprintf(strbuf, "%s Send to %s => %s\n", buffer, inet_ntoa(addr_remote.sin_addr), macbuf);
						//printf("%s Send to %s => %s\n", buffer, inet_ntoa(addr_remote.sin_addr), macbuf);
						printf("%s", strbuf);
						if ((fp = fopen(MACSERVERLOG, "a+")) != NULL)
						{
							fputs(strbuf, fp);
							fclose(fp);
						}
					}
				}
				else
				{
					printf("getmac error!\n");
				}
			}
		}
	}
	close(sockfd);
#if defined(__WIN32__) || defined(_WIN32)
	WSACleanup();
#endif
	return (0);
}

