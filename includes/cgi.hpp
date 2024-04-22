#ifndef CGI_HPP
# define CGI_HPP

# include <string>
# include <iostream>
# include <unistd.h>
# include <map>
# include <sys/wait.h>
# include <fcntl.h>
# include "utils.hpp"
# include "Server.hpp"

# define BUFFER_SIZE 30000

class Server;

class Cgi {
	private:
		std::string _header;
		std::string _filePath;
		Server *_serv;

		char **getExecveArgs(void);
		char **setEnv(std::string savedRoot);
		std::string getPort(void);
		std::string getMethod(void);
		void insertPathInfo(std::map<std::string, std::string> & envMap, std::string root);
		std::string getScriptRelative(std::string root);
		std::string getRemoteHost(void);
		void addServerNames(std::map<std::string, std::string> & envMap);
		void addHeaderField(std::map<std::string, std::string> & envMap, std::string key, std::string headerKey);

	public:
		Cgi(std::string header, std::string filePath, Server *serv, std::string savedRoot);
		~Cgi(void);
};

#endif
