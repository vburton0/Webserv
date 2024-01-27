#ifndef WEBSERV_HPP
#define WEBSERV_HPP

#include <list>
#include "Server.hpp"

class Webserv {
public:
    Webserv() {}
    ~Webserv() {}
    std::list<Server> servers;
    std::vector<int> serverSockets;
    std::vector<int> clientSockets;
};

#endif // WEBSERV_HPP