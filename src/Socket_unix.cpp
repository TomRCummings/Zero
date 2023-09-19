#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>

#include "SocketWrapper.h"

struct addrinfo hints;
struct addrinfo *results, *rp;

int initialize() {
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = 0;
    hints.ai_protocol = 0;
    return 0;
}

void cleanup() {
    return;
}

std::shared_ptr<SocketWrapper> openSocket() {
    int newSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (newSocket < 0) {
        //printf("socket failed with error code: %ld\n", newSocket);
        newSocket = -1;
    }
    return std::shared_ptr<SocketWrapper>(new SocketWrapper(newSocket));
}

std::shared_ptr<SocketWrapper> connectSocket(const char* serverName, const char* port) {
    int sfd;
    int addrResult = getaddrinfo(serverName, port, &hints, &results);
    for (rp = results; rp != NULL; rp = rp->ai_next) {
        sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (connect(sfd, rp->ai_addr, rp->ai_addrlen) != -1) {
            break;
        }
        close(sfd);
    }
    freeaddrinfo(results);
    if (rp == NULL) {
        return std::shared_ptr<SocketWrapper>(new SocketWrapper(-1));
    } else {
        return std::shared_ptr<SocketWrapper>(new SocketWrapper(sfd));
    }
}

int send(std::shared_ptr<SocketWrapper> socket, const char* bufferToSend) {
    return send(socket->GetSocketDescriptor(), bufferToSend, strlen(bufferToSend), 0);
}

int shutdown(std::shared_ptr<SocketWrapper> socket, int flag) {
    int shutdownResult = shutdown(socket->GetSocketDescriptor(), flag);
    if (shutdownResult < 0) {
        return -1;
    }
    return 0;
}

int receive(std::shared_ptr<SocketWrapper> socket, char* buffer, int bufferSize) {
	return recv(socket->GetSocketDescriptor(), buffer, bufferSize, 0);
}

SocketWrapper::~SocketWrapper() {
	if (_socketDescriptor >= 1) {
		close(_socketDescriptor);
	}
}