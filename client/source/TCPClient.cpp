#include <iostream>
#include <exception>
#include <string>
#include <regex>
#include <thread>

#include <sys/socket.h>	// socket, bind, listen
#include <sys/types.h>	// socket, bind, listen
#include <netinet/in.h>	// struct sockaddr_in
#include <arpa/inet.h>	// inet_addr
#include <unistd.h>	// close

#include "common.h"

#include "TCPClient.h"

void receiveHandler(int fd) {

	int ret { 0 };
	std::string feedback { };
	char buffer[BUFFER_SIZE] = { 0 };

	while (true) {
		ret = recv(fd, buffer, BUFFER_SIZE, 0);
		if (ret < 0) {
			throw std::runtime_error("Something went wrong.");
		}
		else if (ret == 0) {
			std::cout << "Disconnected." << std::endl;
		}
		else {
			feedback.append(buffer);
			if (buffer[ret - 1] == '\n') {
				(void)feedback.pop_back();
				if (feedback == "QUIT"
				 || feedback == "EXIT"
				 || feedback == "DISCONNECT")
					break;
				else {
					std::cout << feedback << std::endl;
				}
				feedback.clear();
				memset(buffer, 0, BUFFER_SIZE);
			}
		}
	}

	return;
}

void TCPClient::connectTo(int port) {

	int ret { 0 };
	struct sockaddr_in saddr { 0 };

	this->fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (fd < 0)
	       throw std::runtime_error("Failed to open socket.");

	std::cout << "Connecting to " << IPADDR << ":"
		  << std::to_string(port) << std::endl;

	saddr = {
		.sin_family = AF_INET,			// IPv4
		.sin_port = htons(port),		// port
	};
	
	ret = inet_pton(AF_INET, IPADDR, &saddr.sin_addr);
	if (ret < 0)
		throw std::runtime_error("Invalid address or address family.");

	ret = connect(fd, (struct sockaddr *)&saddr, sizeof(struct sockaddr));
	if (ret < 0)
		throw std::runtime_error("Could not connect to server.");

	else if (!ret)
		std::cout << "Connected..." << std::endl;

	receiveThread = std::thread(receiveHandler, fd);

	return;
}

void TCPClient::start() {

	int ret { 0 };
	std::string input {};
	std::smatch match;

	while (true) {
		std::getline(std::cin, input);
		input = trim(input);

		if (REGEX("^CONNECT\\s+([0-9]+)\\s+([a-zA-Z0-9_]+)$")) {
			try {
				this->connectTo(
					std::stoi(match[1].str())
				);
				input = "SETNAME " + match[2].str();
			}
			catch (const std::exception& e) {
				std::cout << e.what() << std::endl;
				continue;
			}
		}
		else if (REGEX("^DISCONNECT$")) {
			if (fd < 0)
				std::cout << "(!) Already disconnected."
					  << std::endl;
		}
		else if (REGEX("^(QUIT|EXIT)$")) {
			if (fd < 0) {
				std::cout << "Exiting..." << std::endl;
				break;
			}
		}

		input.append("\n");
		ret = send(fd, input.c_str(), input.length(), 0);

		if (input == "QUIT\n" || input == "EXIT\n") {
			receiveThread.join();
			break;
		}
		else if (input == "DISCONNECT\n") {
			receiveThread.join();
			continue;
		}
	}

	return;
}
