#ifndef TCP_CLIENT_H
#define TCP_CLIENT_H

#include <string>
#include <thread>

struct TCPClient {

	std::string name;
	int fd = -1;

	TCPClient() {};

	void start();
	void connectTo(int port);

	~TCPClient() {};

private:
	std::thread receiveThread;
};

#endif
