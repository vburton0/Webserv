#ifndef CGI_HPP
# define CGI_HPP

# include <string>
# include <iostream>
# include <unistd.h>
# include <map>

class Cgi {
private:
    std::string _uri;
	std::string _method;
    std::string _requestBody;
    int _clientSocket;
    std::map<std::string, std::string> _environment;

    void setupEnvironment();
    void executeScript();
	std::string findScriptPath(const std::string& uri);

public:
    Cgi(const std::string& uri, const std::string& requestBody, int clientSocket, const std::string& method);
    ~Cgi();
    void execute();
};

#endif // CGI_HPP

