#include <iostream>
#include <exception>
#include <cstring>
#include <cerrno>

#include <sys/socket.h>	// socket, bind, listen
#include <sys/types.h>	// socket, bind, listen
#include <netinet/in.h>	// struct sockaddr_in
#include <arpa/inet.h>	// inet_addr

#include "tcp-server.h"
#include "client-data.h"
#include "topic.h"

#define strerror_s std::string(std::strerror(errno)) + \
	" (" + std::to_string(errno) + ")"

#define IPADDR "127.0.0.1"
#define BACKLOG 3	// max. number of pending connections in listen()
#define TIMEOUT 120	// timeout in select()

void TCPServer::start() const {
	int fd, max_fd, ret, client_fd;
	struct sockaddr_in saddr;
	fd_set sock_fds, sock_fds_temp;
	struct timeval timeout{ TIMEOUT, 0 }; // 2min

	memset(&saddr, 0, sizeof(saddr));
	saddr = {
		.sin_family = AF_INET,				// IPv4
		.sin_port = htons(this->port),			// port
		.sin_addr { .s_addr = inet_addr(IPADDR) }	// localhost
	};

	fd = socket(
		AF_INET,	// IPv4
		SOCK_STREAM,	// TCP
		IPPROTO_TCP	// TCP
	);
	if (!fd)
		throw std::runtime_error("Could not open socket.");

	errno = 0;
	if (bind(fd, (struct sockaddr *)&saddr, sizeof(saddr)) < 0)
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
		memcpy(&sock_fds, &sock_fds_temp, sizeof(sock_fds));

		ret = select(max_fd + 1, &sock_fds_temp, NULL, NULL, &timeout);

		if (ret < 0)
			throw std::runtime_error(
				"Select failed (" +
				std::to_string(ret) + ")."
				);

		else if (!ret)
			throw std::runtime_error("Select timeout.");

		for (int i = 0; i <= max_fd; i++) {

			if (ret == 0)
				break;

			if (!FD_ISSET(i, &sock_fds_temp))
				continue;

			if (i != fd) {
				this->receiveData();
				continue;
			}

			try {
				client_fd = this->acceptClient(fd);
				max_fd = max_fd > client_fd ?: client_fd;
				FD_SET(client_fd, &sock_fds);
			}
			catch (std::exception& ex) {
				throw std::runtime_error(
					"Could not accept connection: "
					+ std::string(ex.what())
				);
			}
		}
	}

	return;
}

int TCPServer::acceptClient(const int& fd) const {

	int client_fd;
	struct sockaddr_in csaddr;
	socklen_t csaddrlen;
	
	errno = 0;
	client_fd = accept(fd, (struct sockaddr *)&csaddr, &csaddrlen);

	if (client_fd < 0)
		throw std::runtime_error("Accept failed: " + strerror_s);

	return client_fd;
}

void TCPServer::receiveData() const {

}

