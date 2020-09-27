#ifndef CLIENT_DATA_H
#define CLIENT_DATA_H

#include <string>

#include <netinet/in.h>

struct ClientData {

	std::string name;
	int socket_fd = -1;
	bool connected = false;

	ClientData(const int& fd): socket_fd(fd) {};

	void addToBuffer(const char* part) noexcept {
		this->buffer.append(std::string(part));
	};

	std::string flushBuffer() noexcept {
		auto buff_old = this->buffer;
		this->buffer.clear();
		return buff_old;
	};

	~ClientData() = default;
	
private:
	std::string buffer;
};

#endif
