#include <iostream>
#include <exception>
#include <string>
#include <regex>

#include <sys/socket.h>	// socket, bind, listen
#include <sys/types.h>	// socket, bind, listen
#include <netinet/in.h>	// struct sockaddr_in
#include <arpa/inet.h>	// inet_addr
#include <unistd.h>	// close

#include "TCPClient.h"

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

	return;
}

/* inspired by answer on SO */
std::string trim(const std::string& string)
{
	size_t first, last;
	
	first = string.find_first_not_of(' ');

	if (std::string::npos == first)
		return string;

	last = string.find_last_not_of(' ');

	return string.substr(first, (last - first + 1));
}

#define REGEX(r) std::regex_match(input, match, std::regex(r))

void TCPClient::start() {

	int ret { 0 };
	std::string input {};
	std::smatch match;

	while (true) {
		std::cout << "> ";
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
			if (fd >= 0) {
				close(this->fd);
				this->fd = -1;
			}
			else {
				std::cout << "(!) Already disconnected."
					  << std::endl;
			}
			continue;
		
		}
		else if (REGEX("^(QUIT|EXIT)$")) {
			return;
		}

		input.append("\n");
		ret = send(fd, input.c_str(), input.length(), 0);
	}

	return;
}
