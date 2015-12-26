#include "Client.h"
int main()
{
	Client client("localhost", "10001");
	std::string message = "";

	while(message.find("/Quit") == std::string::npos)
	{
		std::getline(std::cin, message);
		client.pushMessage(message);
		client.update();

		auto recieved = client.getMessages();

		while(!recieved.empty())
		{
			std::cout << recieved.front() << std::endl;
			recieved.pop();
		}
	}

}
