#include "Actions.h"

std::function<Actions::Event(std::string, Client)> Actions::getAction(std::string command)
{
    std::string key = getValidActionString(command);
    if(actions.count(key) == 0)
    {
        return nullptr;
    }

    return actions[key];
}

bool Actions::setAction(std::string command, std::function<Actions::Event(std::string, Client)> action)
{
    if(actions.count(command) != 0)
    {
        return false;
    }

    actions[command] = action;
    return true;
}

bool Actions::containsAction(std::string command)
{
    for(auto val : actions)
    {
        if(command.find(val.first) != std::string::npos)
        {
            return true;
        }
    }
    return false;
}

std::string Actions::getValidActionString(std::string command)
{
    for(auto val : actions)
    {
        size_t pos = command.find(val.first);
        if(pos != std::string::npos)
        {
            return val.first;
        }
    }
    return "";
}
