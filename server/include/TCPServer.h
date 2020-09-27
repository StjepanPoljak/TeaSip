#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <unordered_map>
#include <memory>

#include <sys/select.h>

#include "ClientData.h"
#include "Topic.h"

struct TCPServer
{
	TCPServer(int port): port(port) {};

	void start();

	int acceptClient(const int& fd) const;
	int removeClient(const int& fd) const;

	void receiveData(const int& fd, fd_set& sock_fds);
	void processData(const int& fd);

	~TCPServer() {
		this->clients.clear();
		this->topics.clear();
	};

private:
	int port;
	std::unordered_map<int, std::unique_ptr<ClientData>> clients;
	std::unordered_map<std::string, Topic> topics;
};

#endif
