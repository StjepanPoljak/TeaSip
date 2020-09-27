#include <iostream>
#include <exception>

#include "TCPServer.h"

int main(int argc, char* argv[]) {

	if (argc < 2) {
		throw std::runtime_error("Please type in port number");
		return 1;
	}

	TCPServer(std::stoi(argv[1])).start();

	return 0;
}
