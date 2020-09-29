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

#include "common.h"

#include "TCPServer.h"
#include "ClientData.h"
#include "Topic.h"

#define strerror_s std::string(std::strerror(errno)) + \
	" (" + std::to_string(errno) + ")"

#define BACKLOG 3	// max. number of pending connections in listen()

void TCPServer::start() {

	int fd, max_fd, ret, client_fd;
	struct sockaddr_in saddr;
	fd_set sock_fds, sock_fds_temp;
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

	// HARDCODE SOME TOPICS
	ADD_TOPIC("topic1");
	ADD_TOPIC("topic2");

	while (true) {

		/* copy sock_fds to sock_fds_temp so we don't confuse
		 * select() if we happen to add another fd to sock_fds */

		memcpy(&sock_fds_temp, &sock_fds, sizeof(sock_fds));

		ret = select(max_fd + 1, &sock_fds_temp, NULL, NULL, NULL);

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
				continue;
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

void TCPServer::checkTopic(const std::string& topic) const {

	if (this->topics.find(topic) == this->topics.end())
		throw std::runtime_error("Topic does not exist.");
}

void sendInvalidTopicError(const int& fd, const std::string& error) {
	(void)send(fd, error.c_str(), error.length(), 0);
}

void TCPServer::processData(const int& fd) {

	std::string input { };
	std::smatch match;
	auto buf = this->clients[fd]->flushBuffer();

	(void)buf.pop_back();
	input = trim(buf);

	if (REGEX("^SETNAME\\s+([a-zA-Z0-9_]+)$")) {
		std::cout << "Set name to: " << match[1].str() << std::endl;
		this->clients[fd]->name = match[1].str();
	}
	else if (REGEX("^SUBSCRIBE\\s+([a-zA-Z0-9_]+)$")) {
		std::cout << "Subscribing to " << match[1].str() << std::endl;
		try {
			this->topics[match[1].str()]->subscribe(fd);
		}
		catch (std::exception& e) {
			sendInvalidTopicError(fd, e.what());
		}
	}
	else if (REGEX("^UNSUBSCRIBE\\s+([a-zA-Z0-9_]+)$")) {
		std::cout << "Unsubscribe from " << match[1].str() << std::endl;
		try {
			this->checkTopic(match[2].str());
			this->topics[match[1].str()]->unsubscribe(fd);
		}
		catch (std::exception& e) {
			sendInvalidTopicError(fd, e.what());
		}
	}
	else if (REGEX("^PUBLISH\\s+([a-zA-Z0-9_]+)\\s+(.*)$")) {
		std::cout << "Publishing." << std::endl;
		try {
			this->checkTopic(match[1].str());
			auto match_str = match[2].str();
			this->topics[match[1].str()]->forEachSubscriber([&match_str](int sfd){
				(void)send(sfd, match_str.c_str(), match_str.length(), 0);
			});
		}
		catch (std::exception& e) {
			sendInvalidTopicError(fd, e.what());
		}
	}
	else if (input == "QUIT" || input == "EXIT" || input == "DISCONNECT") {
		(void)send(fd, "QUIT\n", 5, 0);
		std::cout << "Informing client to quit." << std::endl;
	}
	else {
		std::string error { "Invalid command.\n" };
		(void)send(fd, error.c_str(), error.length(), 0);
		std::cout << "Invalid command." << std::endl;
	}

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

		if (buffer[data_len - 1] == '\n') {
			memset(buffer, 0, BUFFER_SIZE);
			this->processData(fd);
		}

		return;
	}

	close(fd);
	FD_CLR(fd, &sock_fds);

	return;
}

