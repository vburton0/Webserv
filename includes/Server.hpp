#ifndef SERVER_HPP
#define SERVER_HPP

#include "cgi.hpp"
#include "Location.hpp"
#include "utils.hpp"
#include <string>
#include <vector>
#include <map>
#include <list>
#include <iostream>
# include <dirent.h>
#include <sstream>
#include <fstream>
# include <set>

class Location;

class Server
{
    private:
        std::string host;
        int port;
        std::string _root;
        std::vector<Location *> _locations;
        std::string _initial_loc;
        std::string _serverType;
        std::list<std::string> _indexFile;
        bool _bodySighted;
        std::map<int, std::string> _errorPages;
        size_t _maxBodySize;
        std::string rootDirectory; // Root directory for static files
        std::string defaultFile; // Default file to serve

        // Other server-wide settings

    public:
        struct RouteConfig {
            std::string path;
            std::string root;
            std::vector<std::string> methods;
            std::string rootDirectory;
            std::string uploadPath;
            bool directoryListing;
            std::string defaultFile;
            std::map<std::string, std::string> cgi; // Extension and CGI path
            // Additional route-specific settings
        };

        std::list<std::string> _serverNames;
        std::list<int> ports;
        int socketFd;
        size_t actualBodySize;
        std::map<std::string, RouteConfig> routes; // Map of route path to configuration
        Server() : port(0), _maxBodySize(0) {}

        // Setters
        void setHost(const std::string& h) { host = h; }
        void setPort(int p) { port = p; }
        void addServerName(const std::string& name) { _serverNames.push_back(name); }
        void setErrorPage(int statusCode, const std::string& path) { _errorPages[statusCode] = path; }
        void setClientMaxBodySize(size_t size) { _maxBodySize = size; }
        void setRoute(const std::string& path, const RouteConfig& route) { routes[path] = route; }
        void setRootDirectory(const std::string& dir) { rootDirectory = dir; }
        void setDefaultFile(const std::string& file) { defaultFile = file; }
        void addPorts(std::set<int> &all_ports, size_t *number_of_ports);

        //Copy
        void checkSetDefault(void);
        void compareBlockInfo(std::string line, std::ifstream & indata);
        void showServerContent(void);
        std::string recvRequest(int flag);
        void sendResponse(const std::string response);
        void sendError(int err_code, std::string errstr);
        std::string checkChunckEncoding(std::string bufstr);
        void analyseRequest(std::string bufstr);
        std::string getPathFromLocations(std::string & loc, int head_offset, std::string method, bool recursive_stop);
        void dirListing(DIR *dir);
        void handleRequest(std::string bufstr, std::ofstream &outfile, size_t expected_size, std::string content);
        void sendErrorMethod(std::vector<std::string> methods);
        void checkForCGI(std::string header, std::string bufstr, int method_offset, std::string method, std::string saved_root);
        std::string getFirstIndexFile(std::string root, std::list<std::string> index_files, bool auto_index);

        // Getters and other utility functions
        const std::string& getHost() const { return host; }
        int getPort() const { return port; }
        static std::list<Server> parseConfigFile(const std::string& configFile);
        
        // Other utility functions
        static bool isFileRequest(const std::string& uri);
        static std::string getFilePath(const std::string& uri);
        static std::string getMimeType(const std::string& filePath);


        class IncompleteServerException : public std::exception
        {
            public:
                const char *what() const throw();
        };
};

#endif // SERVER_HPP