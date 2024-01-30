#include "../includes/Server.hpp"
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

bool hasExtension(const std::string& filename, const std::string& extension) {
    size_t dotPos = filename.rfind('.');
    if (dotPos == std::string::npos) {
        return false; // No dot found in filename
    }
    return filename.substr(dotPos) == extension;
}

void parseHttpRequest(const std::string& request, std::string& method, std::string& uri, std::string& httpVersion, std::map<std::string, std::string>& headers) {
    std::istringstream requestStream(request);
    std::string line;

    // Get the request line
    std::getline(requestStream, line);
    std::istringstream requestLineStream(line);
    requestLineStream >> method >> uri >> httpVersion;

    std::cout << "\n\n\n\n\nRequest:\n" << request << "\n\n\n\n\n" << std::endl;  // DEBUG
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
    // Try with Poll
    std::vector<struct pollfd> fds;

    // Add all server sockets to the fds vector for monitoring incoming connections
    for (int serverSocket : webserv.serverSockets) {
        struct pollfd server_fd = { .fd = serverSocket, .events = POLLIN, .revents = 0 };
        fds.push_back(server_fd);
    }

    // Optionally, add existing client sockets to the fds vector
    for (int clientSocket : webserv.clientSockets) {
        struct pollfd client_fd = { .fd = clientSocket, .events = POLLIN, .revents = 0 };
        fds.push_back(client_fd);
    }

    // 5. Main loop for handling connections
    // fd_set readfds;
    // int max_sd;

    int timeout = -1; // Timeout in milliseconds, -1 for no timeout

while (true) {
    int ret = poll(fds.data(), fds.size(), timeout);

    if (ret < 0) {
        std::cerr << "poll error: " << std::strerror(errno) << std::endl;
        break;
    } else if (ret == 0) {
        // Timeout occurred, handle if needed
        continue;
    }

    // Iterate over fds to check which ones are ready
    for (auto &fd : fds) {
        if (fd.revents & POLLIN) {
            if (std::find(webserv.serverSockets.begin(), webserv.serverSockets.end(), fd.fd) != webserv.serverSockets.end()) {
                // Handle new connections on server sockets
                struct sockaddr_in clientAddr;
                socklen_t clientAddrLen = sizeof(clientAddr);
                int newClientSocket = accept(fd.fd, (struct sockaddr *)&clientAddr, &clientAddrLen);
                if (newClientSocket >= 0) {
                    struct pollfd new_fd = { .fd = newClientSocket, .events = POLLIN, .revents = 0 };
                    fds.push_back(new_fd);
                    webserv.clientSockets.push_back(newClientSocket);
                }
            } else {
                // This is a client socket, handle data from client
                std::string accumulatedRequest;
                char buffer[1024];
                ssize_t bytesRead;

                bytesRead = read(fd.fd, buffer, sizeof(buffer) - 1);
                if (bytesRead > 0) {
                    buffer[bytesRead] = '\0'; // Null-terminate the string
                    accumulatedRequest += buffer;

                    // Check if the end of headers (or entire request) is reached
                    if (accumulatedRequest.find("\r\n\r\n") != std::string::npos) {
                        // Process the request
                        std::cout << "Received: " << buffer << std::endl;

                    // Check if the end of headers is reached
                    if (accumulatedRequest.find("\r\n\r\n") != std::string::npos) {
                        // Now you have the complete headers in clientBuffers[clientSocket]
                        // Parse the request here
                        std::string method, uri, httpVersion;
                        std::map<std::string, std::string> headers;
                        parseHttpRequest(accumulatedRequest, method, uri, httpVersion, headers);
                    
                    std::cout << "Method: " << method << ", URI: " << uri << ", HTTP Version: " << httpVersion << std::endl;
                    
                    
                    // TODO: Parse HTTP request and respond
                    // std::istringstream requestStream(buffer);
                    // std::string requestLine;
                    // std::getline(requestStream, requestLine); // Get the request line

                    // std::string method, uri, httpVersion;
                    // std::istringstream requestLineStream(requestLine);
                    // requestLineStream >> method >> uri >> httpVersion;

                    // // Output the parsed request line (for debugging)
                    // std::cout << "Method: " << method << ", URI: " << uri << ", HTTP Version: " << httpVersion << std::endl;

                    // Parse headers (a simplistic approach)
                    // std::string headerLine;
                    // while (std::getline(requestStream, headerLine) && headerLine != "\r") {
                    //     std::cout << "Header: " << headerLine << std::endl;
                    //     // Further processing of headers as needed
                    // }

                    // Test to see what's de method
                    std::cout << "HTTP Method: " << method << std::endl;

                    // If POST, read the body based on Content-Length (this is a simplistic approach)
                    if (method == "POST") {
                        // TODO: Handle POST request body
                    }
                    if (method == "GET") {
                        std::string targetUri = uri;
                        if (uri == "/") {
                            targetUri = "/index.html";  // Serve index.html for the root URI
                        }

                        if (Server::isFileRequest(targetUri)) {
                            std::cout << "File request: " << targetUri << std::endl;
                            std::string filePath = Server::getFilePath(targetUri);
                            std::ifstream file(filePath.c_str(), std::ios::binary);
                            std::cout << "File path: " << filePath << std::endl;
                            

                            if (file) {
                                std::ostringstream ss;
                                ss << file.rdbuf();
                                std::string fileContent = ss.str();
                                
                                // Old response
                                //std::string response = "HTTP/1.1 200 OK\r\nContent-Type: " + Server::getMimeType(filePath) + "\r\n\r\n" + fileContent;
                                // write(clientSocket, response.c_str(), response.length());
                                
                                std::string responseHeader = "HTTP/1.1 200 OK\r\nContent-Type: " + Server::getMimeType(filePath) + "\r\nContent-Length: " + std::to_string(fileContent.length()) + "\r\n\r\n";
                                write(fd.fd, responseHeader.c_str(), responseHeader.length());

                                write(fd.fd, fileContent.c_str(), fileContent.length());

                            } else {
                                std::string response = "HTTP/1.1 404 Not Found\r\n\r\n";
                                write(fd.fd, response.c_str(), response.length());
                            }
                        }
                    }
            }
        }
                } else if (bytesRead == 0) {
                    // Client disconnected
                    std::cout << "Client disconnected, socket: " << fd.fd << std::endl;
                    close(fd.fd);
                    fd.fd = -1;
                } else {
                    if (errno != EWOULDBLOCK) {
                        std::cerr << "read error: " << std::strerror(errno) << std::endl;
                        close(fd.fd);
                        fd.fd = -1;
        }
                }


                
            }
        }
        // Remove closed sockets from fds and webserv.clientSockets
        fds.erase(std::remove_if(fds.begin(), fds.end(), [](const struct pollfd &pfd) { return pfd.fd == -1; }), fds.end());
        webserv.clientSockets.erase(std::remove_if(webserv.clientSockets.begin(), webserv.clientSockets.end(),
                                                [&fds](int socket) { return std::none_of(fds.begin(), fds.end(), [socket](const struct pollfd &pfd) { return pfd.fd == socket; }); }),
                                    webserv.clientSockets.end());
    }
    return 0;
}
}