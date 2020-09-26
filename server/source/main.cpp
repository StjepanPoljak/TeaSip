#include <iostream>
#include <exception>

#include "tcp-server.h"

int main(int argc, char* argv[]) {

	if (argc < 2) return 1;



	TCPServer(std::stoi(argv[1])).start();

	return 0;
}
