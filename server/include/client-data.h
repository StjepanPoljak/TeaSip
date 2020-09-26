#ifndef CLIENT_DATA_H
#define CLIENT_DATA_H

#include <string>

#include <netinet/in.h>

struct ClientData {

	std::string name;
	const int socket_fd;

	ClientData(const int& fd): socket_fd(fd) {};

	void addToBuffer(const char* part) noexcept {
		this->buffer.append(std::string(part));
	};

	~ClientData() = default;
	
private:
	std::string buffer;
};

#endif
