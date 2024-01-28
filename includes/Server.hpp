#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>
#include <vector>
#include <map>
#include <list>

class Server {
public:
    struct RouteConfig {
        std::string path;
        std::vector<std::string> methods;
        std::string rootDirectory;
        std::string uploadPath;
        bool directoryListing;
        std::string defaultFile;
        std::map<std::string, std::string> cgiHandlers; // Extension and CGI path
        // Additional route-specific settings
    };

    Server() : port(0), clientMaxBodySize(0) {}

    // Setters
    void setHost(const std::string& h) { host = h; }
    void setPort(int p) { port = p; }
    void addServerName(const std::string& name) { serverNames.push_back(name); }
    void setErrorPage(int statusCode, const std::string& path) { errorPages[statusCode] = path; }
    void setClientMaxBodySize(size_t size) { clientMaxBodySize = size; }
    void addRoute(const RouteConfig& route) { routes.push_back(route); }

    // Getters and other utility functions
    const std::string& getHost() const { return host; }
    int getPort() const { return port; }
    static std::list<Server> parseConfigFile(const std::string& configFile);
    
    // Other utility functions
    static bool isFileRequest(const std::string& uri);
    static std::string getFilePath(const std::string& uri);
    static std::string getMimeType(const std::string& filePath);

private:
    std::string host;
    int port;
    std::vector<std::string> serverNames;
    std::map<int, std::string> errorPages;
    size_t clientMaxBodySize;
    std::vector<RouteConfig> routes;

    // Other server-wide settings

    // Private methods for internal processing
    // ...
};

#endif // SERVER_HPP