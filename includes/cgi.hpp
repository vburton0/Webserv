#ifndef CGI_HPP
# define CGI_HPP

# include <string>
# include <iostream>
# include <unistd.h>
# include <map>
# include <sys/wait.h>
# include "utils.hpp"
# include "Server.hpp"

# define BUFFER_SIZE 30000

class Server;

class Cgi {
	private:
		std::string _header;
		std::string _file_path;
		Server *_serv;

		char **getExecveArgs(void);
		char **setEnv(std::string saved_root);
		std::string get_port(void);
		std::string getMethod(void);
		void insertPathInfo(std::map<std::string, std::string> & env_map, std::string root);
		std::string getScriptRelative(std::string root);
		std::string getRemoteHost(void);
		void add_server_names(std::map<std::string, std::string> & env_map);
		void addHeaderField(std::map<std::string, std::string> & env_map, std::string key, std::string header_key);

	public:
		Cgi(std::string header, std::string file_path, Server *serv, std::string saved_root);
		~Cgi(void);
};

#endif
