#ifndef WEBSERV_HPP
# define WEBSERV_HPP

# include "Server.hpp"
# include <string>
# include <vector>
# include <map>
# include <list>
# include <iostream>
# include <sstream>
# include <sys/socket.h>
# include <netinet/in.h>
# include <poll.h>
# include <unistd.h>

class Server;

class Webserv {
public:
    Webserv();
    ~Webserv();
    std::list<Server *> servers;
    std::vector<int> serverSockets;
    std::vector<int> clientSockets;

    // Methods
    void init(std::string configFile);
    void showServersContent(void);
	void launchServers(void);
	Server *getPollingServer(uint16_t sin_port);





class InvalidFileException : public std::exception
	{
		public:
			const char *what() const throw();
	};

	class InvalidFileExtensionException : public std::exception
	{
		public:
			const char *what() const throw();
	};

	class InvalidFileContentException : public std::exception
	{
		public:
			const char *what() const throw();
	};

	class MissingDefault404Exception : public std::exception
	{
		public:
			const char *what() const throw();
	};

	class SystemCallException : public std::exception
	{
		public:
			const char *what() const throw();
	};

	class QuickerReturnException : public std::exception
	{
		public:
			const char *what() const throw();
	};
};

#endif // WEBSERV_HPP