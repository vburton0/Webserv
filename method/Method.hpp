#ifndef METHOD_HPP
# define METHOD_HPP

# include <string>
# include <sstream>
# include <fstream>
# include <filesystem>
# include <unistd.h>
# include <sys/socket.h>
# include "../includes/Server.hpp"
# include "../CGI/Cgi.hpp"

class Method {
private:
	void sendFileResponse(const std::string& filePath, int clientSocket);
	//void handlePost(const std::string& uri, const std::string& requestBody);
	void handleCgi(const std::string& uri, const std::string& requestBody, int clientSocket, const std::string& method);
	void sendErrorResponse(int clientSocket, int errorCode, const std::string& message);
public:
    Method();
    Method(const Method& other);
    ~Method();

    // Opérateur d'affectation
    Method& operator=(const Method& other);

    // Méthode GET
    void get(const std::string& uri, int clientSocket);;

    // Méthode POST
    void post(const std::string& uri, int clientSocket);

    // Méthode DELETE
    void deleteRequest(const std::string& uri, int clientSocket);

};

#endif // METHOD_HPP

