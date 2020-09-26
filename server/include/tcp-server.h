#ifndef TCPSERVER_H
#define TCPSERVER_H

struct TCPServer
{
	TCPServer(int port): port(port) {};

	void start() const;

	int acceptClient(const int& fd) const;

	void receiveData() const;

	~TCPServer() {};

private:
	int port;
};

#endif
