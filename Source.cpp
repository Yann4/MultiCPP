#include "Server.h"
#include <iostream>
#include <string>
int main()
{
	Server server;
	std::string val;
	std::cin >> val;
	server.shutdownServer();
	std::cout << "Shutdown" << std::endl;
}
