#include "../includes/Server.hpp"
#include <sstream>
#include <fstream>

std::list<Server> Server::parseConfigFile(const std::string& filename) {
    std::list<Server> servers;
    std::ifstream configFile(filename);
    if (!configFile.is_open()) {
        throw std::runtime_error("Failed to open configuration file: " + filename);
    }

    std::string line;
    Server currentServer;
    bool inServerBlock = false;

    while (std::getline(configFile, line)) {
        std::istringstream iss(line);
        std::string key;
        iss >> key;

        if (key == "server") {
            if (inServerBlock) {
                servers.push_back(currentServer);
                currentServer = Server();
            }
            inServerBlock = true;
        } else if (inServerBlock && key == "listen") {
        std::string hostPortStr;
        iss >> hostPortStr; // Read the host:port part

        size_t colonPos = hostPortStr.find(':');
        if (colonPos == std::string::npos) {
            throw std::runtime_error("Invalid format for listen directive");
        }

        std::string host = hostPortStr.substr(0, colonPos);
        int port = std::atoi(hostPortStr.substr(colonPos + 1).c_str());
        currentServer.setHost(host);
        currentServer.setPort(port);
        } else if (inServerBlock && key == "server_names") {
            std::string name;
            while (iss >> name) {
                currentServer.addServerName(name);
            }
        }
        // Continue parsing other directives...

        if (inServerBlock && line == "}") {
            servers.push_back(currentServer);
            currentServer = Server();
            inServerBlock = false;
        }
    }

    if (inServerBlock) {
        servers.push_back(currentServer);
    }

    return servers;
}