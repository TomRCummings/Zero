#pragma once

#include <memory>

int initialize();

void cleanup();

class SocketWrapper {
public:
	SocketWrapper(int socketDescriptor): _socketDescriptor(socketDescriptor) {}
	~SocketWrapper();

	int GetSocketDescriptor() const { return _socketDescriptor; }

private:
	SocketWrapper(const SocketWrapper&);

	int _socketDescriptor;
};

std::shared_ptr<SocketWrapper> openSocket();

std::shared_ptr<SocketWrapper> connectSocket(const char* serverName, const char* port);

int send(std::shared_ptr<SocketWrapper> socket, const char* bufferToSend);

int shutdown(std::shared_ptr<SocketWrapper> socket, int flag);

int receive(std::shared_ptr<SocketWrapper> socket, char* buffer, int bufferSize);