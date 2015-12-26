#pragma once
#include <map>
#include <functional>
#include <string>
#include "Structs.h"

namespace Actions
{
    enum class Event : unsigned int{NAME, SPEAK, ENTER, HELP, QUIT};

    static std::map<std::string, std::function<Event(std::string, Client)>> actions;

    std::function<Event(std::string, Client)> getAction(std::string command);
    bool setAction(std::string command, std::function<Event(std::string, Client)> action);

    bool containsAction(std::string command);
    std::string getValidActionString(std::string command);
};
