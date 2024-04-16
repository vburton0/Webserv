#include <iostream>
#include <filesystem>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <sstream>
#include <fstream>
#include <poll.h>
#include <vector>
#include <algorithm>
#include "../includes/Webserv.hpp"


void parseHttpRequest(const std::string& request, std::string& method, std::string& uri, std::string& httpVersion, std::map<std::string, std::string>& headers) {
    std::istringstream requestStream(request);
    std::string line;

    // Get the request line
    std::getline(requestStream, line);
    std::istringstream requestLineStream(line);
    requestLineStream >> method >> uri >> httpVersion;

    // Read headers
    while (std::getline(requestStream, line) && line != "\r") {
        std::istringstream headerLineStream(line);
        std::string key, value;
        if (std::getline(headerLineStream, key, ':')) {
            std::getline(headerLineStream, value);
            if (!value.empty() && value[0] == ' ') {
                value.erase(0, 1); // Remove leading space
            }
            headers[key] = value;
        }
    }
}

int main(int ac, char **av) {
    // Config File and args handler
    std::string configFile;
    if (ac == 1) {
        configFile = "config/default.conf";
    } else if (ac == 2) {
        configFile = av[1];
    } else {
        std::cerr << "Usage: ./webserv [configFile]" << std::endl;
        return 1;
    }


    // Parse the configuration file
    Webserv webserv;
    try {
        webserv.init(configFile);
        webserv.showServersContent();
        webserv.launchServers();
    } catch (const std::exception& e) {
        std::cerr << "Error parsing configuration file: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}