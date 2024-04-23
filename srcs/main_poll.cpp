#include <filesystem>
#include <arpa/inet.h>
#include <cstring>
#include <algorithm>
#include "../includes/Webserv.hpp"

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