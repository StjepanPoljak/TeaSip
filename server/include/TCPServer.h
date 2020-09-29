#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <unordered_map>
#include <memory>

#include <sys/select.h>

#include "ClientData.h"
#include "Topic.h"

#define ADD_TOPIC(name) this->topics[name] = std::unique_ptr<Topic>(new Topic { name })

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
	std::unordered_map<std::string, std::unique_ptr<Topic>> topics;
	void checkTopic(const std::string& topic) const;
};

#endif
