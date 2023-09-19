#include <WinSock2.h>
#include <WS2tcpip.h>
#include <Windows.h>
#include <stdlib.h>
#include <stdio.h>

#include "SocketWrapper.h"

WSAData wsaData;
struct addrinfo hints;

int initialize() {
	//printf("Initialization starting\n");
	int startupResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (startupResult != 0) {
		//printf("WSAStartup failed with error: %d\n", startupResult);
		return 1;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	return 0;
}

void cleanup() {
	WSACleanup();
}

std::shared_ptr<SocketWrapper> openSocket() {
	int newSocket = socket(hints.ai_family, hints.ai_socktype, hints.ai_protocol);
	if (newSocket == INVALID_SOCKET) {
		//printf("socket failed with error: %ld\n", WSAGetLastError());
		WSACleanup();
		newSocket = -1;
	}
	return std::shared_ptr<SocketWrapper>(new SocketWrapper(newSocket));
}

std::shared_ptr<SocketWrapper> connectSocket(const char* serverName, const char* port) {
	struct addrinfo *results, *ptr;
	int sfd;
	int addrResult = getaddrinfo(serverName, port, &hints, &results);
	bool ableToConnect = false;
	if (addrResult != 0) {
		//printf("getaddrinfo failed with error: %d\n", addrResult);
		WSACleanup();
		return 1;
	}

	for (ptr = results; ptr != NULL; ptr = ptr->ai_next) {
		//Connect to server
		sfd = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
		int connectResult = connect(sfd, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (connectResult == SOCKET_ERROR) {
			//printf("connection failed with error: %ld\n", WSAGetLastError());
			continue;
		}
		else {
			ableToConnect = true;
			break;
		}
	}

	freeaddrinfo(results);

	if (ableToConnect) {
		return std::shared_ptr<SocketWrapper>(new SocketWrapper(sfd));
	}
	else {
		//printf("Unable to connect to server!\n");
		return std::shared_ptr<SocketWrapper>(new SocketWrapper(-1));
	}
}

int send(std::shared_ptr<SocketWrapper> socket, const char* bufferToSend) {
	int sendResult = send((SOCKET)socket->GetSocketDescriptor(), bufferToSend, (int)strlen(bufferToSend), 0);
	if (sendResult == SOCKET_ERROR) {
		//printf("send failed with error: %d\n", WSAGetLastError());
		return 1;
	}
	//printf("Bytes Sent: %ld\n", sendResult);

	return 0;
}

int shutdown(std::shared_ptr<SocketWrapper> socket, int flag) { //FIXME: The flag field is hard-coded at this moment.
	int shutdownResult = shutdown((SOCKET)socket->GetSocketDescriptor(), SD_SEND);
	if (shutdownResult == SOCKET_ERROR) {
		//printf("shutdown failed with error: %d\n", WSAGetLastError());
		return 1;
	}
	return 0;
}

int receive(std::shared_ptr<SocketWrapper> socket, char* buffer, int bufferSize) {
	int receiveResult = recv((SOCKET)socket->GetSocketDescriptor(), buffer, bufferSize, 0);
	//printf("received %d bytes\n", receiveResult);
	if (receiveResult < 0) {
		//printf("receive failed with error: %d\n", WSAGetLastError());
	}
	return receiveResult;
}

SocketWrapper::~SocketWrapper() {
	if (_socketDescriptor != INVALID_SOCKET) {
		closesocket((SOCKET)_socketDescriptor);
	}
}