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
    bool inRouteBlock = false;
    std::string currentRoutePath;
    RouteConfig currentRouteConfig;

    while (std::getline(configFile, line)) {
        // Ignores comments and empty lines
        size_t commentPos = line.find('#');
        if (commentPos != std::string::npos) {
            line = line.substr(0, commentPos);
        }
        if (line.empty() || line[0] == '#') continue;

        std::istringstream iss(line);
        std::string key;
        iss >> key;

        std::cout << "Line : " << line << std::endl;

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
        } else if (inServerBlock && key == "error_page") {
            int errorCode;
            std::string errorPage;
            if (!(iss >> errorCode >> errorPage)) {
                throw std::runtime_error("Invalid format for error_page directive");
            }
            currentServer.setErrorPage(errorCode, errorPage);
        } else if (inServerBlock && key == "client_body_size") {
            int size;
            if (!(iss >> size)) {
                throw std::runtime_error("Invalid format for client_body_size directive");
            }
            currentServer.setClientMaxBodySize(size);
        }else if (inServerBlock && key == "root") {
            iss >> currentServer.rootDirectory;
        } else if (inServerBlock && key == "index") {
            iss >> currentServer.defaultFile;
        } else if (inServerBlock && key == "route") {
            iss >> currentRoutePath;
            inRouteBlock = true;
            currentRouteConfig = RouteConfig();
        } else if (inRouteBlock) {
            if (key == "methods") {
                std::string method;
                while (iss >> method) {
                    currentRouteConfig.methods.push_back(method);
                }
            } else if (key == "root") {
                iss >> currentRouteConfig.root;
            } else if (key == "directory_listing") {
                std::string listing;
                iss >> listing;
                currentRouteConfig.directoryListing = (listing == "on");
            } else if (key == "default_file") {
                iss >> currentRouteConfig.defaultFile;
            } else if (key == "cgi") {
                std::string ext, cgiPath;
                iss >> ext >> cgiPath;
                currentRouteConfig.cgi[ext] = cgiPath;
            } else if (key == "upload_path") {
                iss >> currentRouteConfig.uploadPath;
            }
        }

        if (inRouteBlock && line == "}") {
            currentServer.setRoute(currentRoutePath, currentRouteConfig);
            inRouteBlock = false;
        }

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

bool Server::isFileRequest(const std::string& uri) {
    size_t lastDotPos = uri.find_last_of(".");
    if (lastDotPos != std::string::npos) {
        // Check if there's an extension
        return uri.find('/', lastDotPos) == std::string::npos; // Ensure no '/' after the dot
    }
    return false;
}

std::string Server::getFilePath(const std::string& uri) {
    const std::string rootDirectory = "resources/Les2canons"; // Change to your static files directory
    return rootDirectory + uri; // Simple concatenation of root directory and URI
}

std::string Server::getMimeType(const std::string& filePath) {
    size_t dotPos = filePath.find_last_of(".");
    if (dotPos != std::string::npos) {
        std::string ext = filePath.substr(dotPos + 1);

        // Basic MIME types
        if (ext == "html" || ext == "htm") return "text/html";
        if (ext == "css") return "text/css";
        if (ext == "js") return "application/javascript";
        if (ext == "json") return "application/json";
        if (ext == "png") return "image/png";
        if (ext == "jpg" || ext == "jpeg") return "image/jpeg";
        if (ext == "gif") return "image/gif";
        if (ext == "svg") return "image/svg+xml";
        if (ext == "txt") return "text/plain";
        // Add more MIME types as needed
    }
    return "application/octet-stream"; // Default MIME type
}