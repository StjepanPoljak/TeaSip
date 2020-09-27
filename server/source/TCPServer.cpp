#include <iostream>
#include <exception>
#include <cstring>
#include <cerrno>
#include <regex>

#include <sys/socket.h>	// socket, bind, listen
#include <sys/types.h>	// socket, bind, listen
#include <netinet/in.h>	// struct sockaddr_in
#include <arpa/inet.h>	// inet_addr
#include <unistd.h>	// close

#include "TCPServer.h"
#include "ClientData.h"
#include "Topic.h"

#define strerror_s std::string(std::strerror(errno)) + \
	" (" + std::to_string(errno) + ")"

#define BACKLOG 3	// max. number of pending connections in listen()
#define TIMEOUT 120	// timeout in select()

void TCPServer::start() {

	int fd, max_fd, ret, client_fd;
	struct sockaddr_in saddr;
	fd_set sock_fds, sock_fds_temp;
	struct timeval timeout{ TIMEOUT, 0 }; // 2min
	int optval = 1;
	static int last_uid = 0;

	saddr = {
		.sin_port = htons(this->port),
		.sin_addr = { .s_addr = INADDR_ANY }
	};

	fd = socket(
		AF_INET,	// IPv4
		SOCK_STREAM,	// TCP
		IPPROTO_TCP	// TCP
	);
	if (!fd)
		throw std::runtime_error("Could not open socket.");

	errno = 0;
	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int)) == -1)
		throw std::runtime_error(
			"Could not set socket options: " +
			strerror_s
			);

	errno = 0;
	if (bind(fd, (struct sockaddr *)&saddr, sizeof(saddr)) == -1)
		throw std::runtime_error(
			"Could not bind socket to address: " +
			strerror_s
			);

	errno = 0;
	if (listen(fd, BACKLOG) < 0)
		throw std::runtime_error(
			"Call to listen() failed" + strerror_s);

	FD_ZERO(&sock_fds);
	FD_SET(fd, &sock_fds);
	max_fd = fd;

	while (true) {

		/* copy sock_fds to sock_fds_temp so we don't confuse
		 * select() if we happen to add another fd to sock_fds */

		memcpy(&sock_fds_temp, &sock_fds, sizeof(sock_fds));

		ret = select(max_fd + 1, &sock_fds_temp, NULL, NULL, &timeout);

		if (ret < 0)
			throw std::runtime_error(
				"Select failed (" +
				std::to_string(ret) + ")." + strerror_s
				);

		else if (!ret)
			throw std::runtime_error("Select timeout.");

		for (int i = 0; i <= max_fd; i++) {

			if (ret == 0)
				break;

			if (!FD_ISSET(i, &sock_fds_temp))
				continue;

			if (i != fd) {
				this->receiveData(i, sock_fds);
				continue;
			}

			try {
				client_fd = this->acceptClient(fd);
				max_fd = max_fd > client_fd ?: client_fd;
				FD_SET(client_fd, &sock_fds);
			}
			catch (std::exception& ex) {
				std::cout << "Could not accept connection: "
					  << ex.what();
				return;
			}
	
			try {
				this->clients[client_fd] =
					std::unique_ptr<ClientData>(
						new ClientData{client_fd}
					);
			}
			catch (std::exception& ex) {
				throw std::runtime_error(
					"Internal inconsistency: client (fd=" +
					std::to_string(fd) + ") already exists."
				);
			}
		}
	}

	return;
}

int TCPServer::acceptClient(const int& fd) const {

	int client_fd;
	struct sockaddr_in csaddr;	// maybe we will need this sometime
	socklen_t csaddrlen;		// in the future
	
	errno = 0;
	client_fd = accept(fd, (struct sockaddr *)&csaddr, &csaddrlen);

	if (client_fd < 0)
		throw std::runtime_error("Accept failed: " + strerror_s);

	std::cout << "Client connected (fd=" << std::to_string(client_fd) << ")" << std::endl;

	return client_fd;
}

void TCPServer::processData(const int& fd) {

	auto buf = this->clients[fd]->flushBuffer();

	(void)buf.pop_back();

	std::cout << buf << std::endl;

	return;
}

void TCPServer::receiveData(const int& fd, fd_set& sock_fds) {

	int data_len;
	char buffer[BUFFER_SIZE] = { 0 };

	data_len = recv(fd, buffer, BUFFER_SIZE, 0);

	if (data_len < 0)
		std::cout << "Error in transmission." << std::endl;

	else if (data_len == 0)
		std::cout << "Client disconnected." << std::endl;

	else {
		this->clients[fd]->addToBuffer(buffer);

		if (buffer[data_len - 1] == '\n')
			this->processData(fd);

		return;
	}

	close(fd);
	FD_CLR(fd, &sock_fds);

	return;
}

