#ifndef TCP_CLIENT_H
#define TCP_CLIENT_H

#include <string>

struct TCPClient {

	std::string name;
	int fd = -1;

	TCPClient() {};

	void start();
	void connectTo(int port);

	~TCPClient() {};
};

#endif
