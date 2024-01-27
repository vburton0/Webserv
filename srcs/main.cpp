#include "../includes/Server.hpp"
#include <iostream>
#include <filesystem>
#include <list>
#include <string>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <sstream>
#include "../includes/Webserv.hpp"

bool hasExtension(const std::string& filename, const std::string& extension) {
    size_t dotPos = filename.rfind('.');
    if (dotPos == std::string::npos) {
        return false; // No dot found in filename
    }
    return filename.substr(dotPos) == extension;
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

    // Check if configFile extension is ".conf"
    if (!hasExtension(configFile, ".conf")) {
        std::cerr << "Invalid config file extension. Must be '.conf'." << std::endl;
        return 1;
    }

    // Parse the configuration file
    Webserv webserv;
    try {
        webserv.servers = Server::parseConfigFile(configFile);
    } catch (const std::exception& e) {
        std::cerr << "Error parsing configuration file: " << e.what() << std::endl;
        return 1;
    }

    // Initialize and run each server based on its configuration
    for (Server& server : webserv.servers) {
        // Setup and run the server
        // 1. Create socket
        int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (serverSocket < 0) {
            perror("socket failed");
            exit(EXIT_FAILURE);
        }

        // 2. Set socket to non-blocking
        fcntl(serverSocket, F_SETFL, O_NONBLOCK);

        // 3. Bind socket to address
        struct sockaddr_in serverAddr;
        memset(&serverAddr, 0, sizeof(serverAddr));
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_addr.s_addr = inet_addr(server.getHost().c_str());
        serverAddr.sin_port = htons(server.getPort());

        if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
            perror("bind failed");
            return 1;
        }

        // 4. Start listening
        if (listen(serverSocket, SOMAXCONN) < 0) {
            perror("listen failed");
            return 1;
        }

        // Add serverSocket to a collection for later use with select/poll
        //  //  //  //  //  //  //  //std::vector<int> serverSockets;
        webserv.serverSockets.push_back(serverSocket);
    }

    // 5. Main loop for handling connections
    fd_set readfds;
    int max_sd;

    while (true) {
        FD_ZERO(&readfds);

        max_sd = -1;
        for (int socket : webserv.serverSockets) {
            FD_SET(socket, &readfds);
            if (socket > max_sd)
                max_sd = socket;
        }

        // Timeout for select (optional)
        struct timeval timeout;
        timeout.tv_sec = 5;
        timeout.tv_usec = 0;

        // Wait for an activity on one of the sockets
        int activity = select(max_sd + 1, &readfds, NULL, NULL, &timeout);

        if ((activity < 0) && (errno != EINTR)) {
            perror("select error");
        }

        // Handle incoming connections and data
        for (int socket : webserv.serverSockets) {
            if (FD_ISSET(socket, &readfds)) {
                // Handle new connections or read data
                struct sockaddr_in clientAddr;
                socklen_t clientAddrLen = sizeof(clientAddr);
                int newSocket = accept(socket, (struct sockaddr *)&clientAddr, &clientAddrLen);
                if (newSocket < 0) {
                    perror("accept");
                    continue;
                }

                // Optionally set the new socket to non-blocking mode
                fcntl(newSocket, F_SETFL, O_NONBLOCK);

                // Add the new socket to the list of client sockets
                webserv.clientSockets.push_back(newSocket);

                // Log or handle new connection
                std::cout << "New connection accepted" << std::endl;
            }
        }

        for (int clientSocket : webserv.clientSockets) {
        if (FD_ISSET(clientSocket, &readfds)) {
            char buffer[1024];
            ssize_t bytesRead = read(clientSocket, buffer, sizeof(buffer) - 1);

            if (bytesRead < 0) {
                if (errno != EWOULDBLOCK) {
                    perror("read error");
                    close(clientSocket); // Close on error
                }
            } else if (bytesRead == 0) {
                std::cout << "Client disconnected" << std::endl;
                close(clientSocket); // Close on disconnection
            } else {
                buffer[bytesRead] = '\0'; // Null-terminate the string
                std::cout << "Received data: " << buffer << std::endl;
                // TODO: Parse HTTP request and respond
                std::istringstream requestStream(buffer);
                std::string requestLine;
                std::getline(requestStream, requestLine); // Get the request line

                std::string method, uri, httpVersion;
                std::istringstream requestLineStream(requestLine);
                requestLineStream >> method >> uri >> httpVersion;

                // Output the parsed request line (for debugging)
                std::cout << "Method: " << method << ", URI: " << uri << ", HTTP Version: " << httpVersion << std::endl;

                // Parse headers (a simplistic approach)
                std::string headerLine;
                while (std::getline(requestStream, headerLine) && headerLine != "\r") {
                    std::cout << "Header: " << headerLine << std::endl;
                    // Further processing of headers as needed
                }

                // If POST, read the body based on Content-Length (this is a simplistic approach)
                if (method == "POST") {
                    // TODO: Handle POST request body
                }
            }
        }
        }
    }


    return 0;
}
