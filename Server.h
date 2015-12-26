#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <map>
#include <functional>

#include "Socket.h"
#include "Structs.h"
#include "Actions.h"

struct Message
{
	std::string data;
	int senderID;

	Message():data(""), senderID(-1){}

	Message(std::string data, int senderID): data(data), senderID(senderID){}
};

class Server
{
private:
    std::vector<Client> clients;
    int ticketNumber = 0;

    std::vector<std::thread> clientThreads;
    std::thread runThread;
    std::atomic<bool> run;
    SOCKET listenSock;

public:
    Server();
    ~Server();

    void shutdownServer();
private:
    std::string getClientIdentifier(int clientID);
    bool getNewClient(SOCKET toListen);
    void handleClient(Client client);

    void broadcast(Message message);
    void whisper(Message message);

    void update();

    Actions::Event changeName(std::string message, Client client);
    Actions::Event clientExit(std::string message, Client client);
    Actions::Event sendHelpText(std::string message, Client client);
    Actions::Event handleMessage(std::string message, Client client);
};
