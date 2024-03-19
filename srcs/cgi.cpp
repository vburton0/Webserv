#include "../includes/cgi.hpp"
#include "../includes/Webserv.hpp"

Cgi::Cgi(std::string header, std::string file_path, Server *serv, std::string saved_root)
		: _header(header), _file_path(file_path), _serv(serv)
{
	std::cout << "Constructor of cgi called" << std::endl << std::endl;
	int body_fd[2];
	if (pipe(body_fd) == -1)
		serv->sendError(500, "500 Internal Server Error");
	size_t index = header.find("Content-Length: ");
	size_t expected_size = std::string::npos;
	if (index == std::string::npos)
	{
		if (!header.compare(0, 5, "POST "))
			serv->sendError(411, "411 Length Required");
	}
	else
	{
		std::istringstream iss(header.substr(index + 16, header.find('\n', index + 16)));
		iss >> expected_size;
		if (iss.fail())
			serv->sendError(412, "412 Precondition Failed");
	}
	if (expected_size != std::string::npos)
	{
		std::string body = getBody(header);
		std::cout << "body size: " << body.size() << " vs expected size: " << expected_size << std::endl;
		if (body.size() != expected_size)
			serv->sendError(412, "412 Precondition Failed");

		const size_t CHUNK_SIZE = 1024; // Adjust the chunk size as needed
		size_t bodyLength = body.size();
		const char* dataPtr = body.c_str();

		for (size_t offset = 0; offset < bodyLength; offset += CHUNK_SIZE) {
			size_t writeSize = std::min(CHUNK_SIZE, bodyLength - offset);
			ssize_t written = write(body_fd[1], dataPtr + offset, writeSize);

			if (written == -1) {
				// Handle error (e.g., log it, close file descriptors, exit)
				perror("write to CGI script failed");
				close(body_fd[0]);
				close(body_fd[1]);
				exit(1); // Or handle the error as appropriate for your application
			}
		}
		std::cout << "on the go ffffff" << std::endl;
	}

	//fork and call cgi

	int pipe_fd[2];
	if (pipe(pipe_fd) == -1)
		serv->sendError(500, "500 Internal Server Error");
	pid_t pid = fork();
	if (pid == -1)
		serv->sendError(500, "500 Internal Server Error");
	if (!pid)
	{
		//setup env
		char **envp = setEnv(saved_root);
		for (int index = 0; envp[index]; index++)
				std::cout << "env line: " << envp[index] << std::endl;

		char **args = getExecveArgs();
		if (dup2(pipe_fd[1], 1) == -1)
			serv->sendError(500, "500 Internal Server Error");
		close(pipe_fd[0]);
		close(pipe_fd[1]);
		if (dup2(body_fd[0], 0) == -1)
			serv->sendError(500, "500 Internal Server Error");
		close(body_fd[0]);
		close(body_fd[1]);
		execve(args[0], args, envp);
		perror(args[0]);
		std::cerr << "execve failure args = ";
		for (int index = 0; args[index]; index++)
		{
			std::cerr << args[index] << ", ";
			delete [] args[index];
		}
		delete [] args;
		for (int index = 0; envp[index]; index++)
			delete [] envp[index];
		delete [] envp;
		std::cerr << std::endl;
		std::cout << "HTTP/1.1 500 Internal Server Error\n\n";
		exit(1);
	}
	close(body_fd[0]);
	close(body_fd[1]);
	close(pipe_fd[1]);

	//read from std::in to send message back to server

	std::string bufstr;
	char buffer[BUFFER_SIZE + 1] = {0};
	ssize_t valread = read(pipe_fd[0], buffer, BUFFER_SIZE);
	if (valread == -1)
		serv->sendError(500, "500 Internal Server Error");
	bufstr += buffer;
	while (valread == BUFFER_SIZE)
	{
		valread = read(pipe_fd[0], buffer, BUFFER_SIZE);
		if (valread == -1)
			serv->sendError(500, "500 Internal Server Error");
		else if (valread)
		{
			buffer[valread] = '\0';
			bufstr += buffer;
		}
		// std::cout << "valread: " << valread << std::endl;
	}
	close(pipe_fd[0]);
	waitpid(pid, NULL, 0);

	std::cout << "RETURN CGI:\n" + bufstr;

	//parsing of read input

	if (bufstr.compare(0, 9, "HTTP/1.1 "))
		serv->sendError(500, "500 Internal Server Error");
	std::istringstream iss(bufstr.substr(9, 3));
	int toint;
	iss >> toint;
	if (iss.fail())
		serv->sendError(500, "500 Internal Server Error");
	if (isErrorCode(toint))
		serv->sendError(toint, bufstr.substr(9, bufstr.find('\n', 9) - 9));

	send(serv->socketFd, bufstr.c_str(), bufstr.size(), 0);

	throw Webserv::QuickerReturnException(); 
}

Cgi::~Cgi(void)
{
}

// ************************************************************************** //
//                                  Private                                   //
// ************************************************************************** //

char **Cgi::getExecveArgs(void)
{
	size_t size = this->_file_path.size();
	if (size > 3)
	{
		if (!this->_file_path.compare(size - 3, 3, ".py"))
		{
			char **res = new char *[3];
			res[0] = ftStrDup("/usr/bin/python3");
			res[1] = ftStrDup(this->_file_path.c_str());
			res[2] = NULL;
			return (res);
		}
		if (!this->_file_path.compare(size - 3, 3, ".pl"))
		{
			char **res = new char *[3];
			res[0] = ftStrDup("/usr/bin/perl");
			res[1] = ftStrDup(this->_file_path.c_str());
			res[2] = NULL;
			return (res);
		}
	}
	char **res = new char *[2];
	res[0] = ftStrDup(this->_file_path.c_str());
	res[1] = NULL;
	return (res);
}

char **Cgi::setEnv(std::string saved_root)
{
	/*
    SERVER_PROTOCOL: HTTP/version.
    SERVER_PORT: TCP port (decimal).
    REQUEST_METHOD: name of HTTP method (see above).
    PATH_INFO: path suffix, if appended to URL after program name and a slash. -> header: METHOD saved_root+PATH_INFO PROTOCOL, can be empty!
    PATH_TRANSLATED: corresponding full path as supposed by server. == saved_root+PATH_INFO
    SCRIPT_NAME: relative path to the program, like /cgi-bin/script.cgi. -> file_path - saved_root
    REMOTE_HOST: host name of the client, unset if server did not perform such lookup.
*   //REMOTE_ADDR: IP address of the client (dot-decimal). -> getaddrinfo ?

    SERVER_NAME: host name of the server, may be dot-decimal IP address. -> serv->serv_names w/ ':'
    CONTENT_TYPE: Internet media type of input data if PUT or POST method are used, as provided via HTTP header. -> Sec-Fetch-Dest
    CONTENT_LENGTH: similarly, size of input data (decimal, in octets) if provided via HTTP header.
	HTTP_ACCEPT
	HTTP_ACCEPT_LANGUAGE
	HTTP_USER_AGENT
	QUERY_STRING: the part of URL after the "?" character. The query string may be composed of *name=value pairs separated with ampersands (such as var1=val1&var2=val2...) when used to submit form data transferred via GET method as defined by HTML application/x-www-form-urlencoded.
	
 *	//HTTP_COOKIE for now
	*/

	std::map<std::string, std::string> env_map;
	env_map.insert(std::pair<std::string, std::string>("SERVER_PROTOCOL", "HTTP/1.1"));
	env_map.insert(std::pair<std::string, std::string>("SERVER_PORT", get_port()));
	env_map.insert(std::pair<std::string, std::string>("REQUEST_METHOD", getMethod()));
	insertPathInfo(env_map, saved_root);
	env_map.insert(std::pair<std::string, std::string>("SCRIPT_NAME", getScriptRelative(saved_root)));
	env_map.insert(std::pair<std::string, std::string>("REMOTE_HOST", getRemoteHost()));
	add_server_names(env_map);
	addHeaderField(env_map, "CONTENT_TYPE", "Content-Type: ");
	addHeaderField(env_map, "CONTENT_LENGTH", "Content-Length: ");
	addHeaderField(env_map, "HTTP_ACCEPT", "Accept: ");
	addHeaderField(env_map, "HTTP_ACCEPT_LANGUAGE", "Accept-Language: ");
	addHeaderField(env_map, "HTTP_USER_AGENT", "User-Agent: ");
	addHeaderField(env_map, "HTTP_COOKIE", "Cookie: ");

	return (mapToArray(env_map));
}

std::string Cgi::get_port(void)
{
	size_t index_start = this->_header.find("Host: ");
	size_t index_end = this->_header.find('\n', index_start);

	std::string host_line = this->_header.substr(index_start, index_end - index_start);
	size_t index_port = host_line.find(':', 5);
	if (index_port == std::string::npos)
		return ("80");
	return (host_line.substr(index_port + 1, host_line.size() - (index_port + 1) - (host_line[host_line.size() - 1] == '\r')));
}

std::string Cgi::getMethod(void)
{
	size_t index = this->_header.find(' ');
	return (this->_header.substr(0, index));
}

void Cgi::insertPathInfo(std::map<std::string, std::string> & env_map, std::string root)
{
	size_t index_start = this->_header.find(' ') + 1;
	size_t index_end = this->_header.find(' ', index_start);
	size_t index_query = this->_header.find('?', index_start);
	if (index_query != std::string::npos && index_query < index_end)
	{
		std::string query_string = this->_header.substr(index_query + 1, index_end - (index_query + 1));
		env_map.insert(std::pair<std::string, std::string>("QUERY_STRING", query_string));
		index_end = index_query;
	}
	std::string path_info = this->_header.substr(index_start + root.size(), index_end - (index_start + root.size()));
	std::string path_translated = this->_header.substr(index_start, index_end - index_start);
	env_map.insert(std::pair<std::string, std::string>("PATH_INFO", path_info));
	env_map.insert(std::pair<std::string, std::string>("PATH_TRANSLATED", path_translated));
}

std::string Cgi::getScriptRelative(std::string root)
{
	return (this->_file_path.substr(root.size(), this->_file_path.size() - root.size()));
}

std::string Cgi::getRemoteHost(void)
{
	size_t index_start = this->_header.find("Host: ");
	size_t index_end = this->_header.find('\n', index_start);

	std::string host_line = this->_header.substr(index_start, index_end - index_start);
	size_t index_port = host_line.find(':', 5);
	if (index_port == std::string::npos)
		return (host_line.substr(6, host_line.size() - 6 - (host_line[host_line.size() - 1] == '\r')));
	return (host_line.substr(6, index_port - 6));
}

void Cgi::add_server_names(std::map<std::string, std::string> & env_map)
{
	std::list<std::string>::iterator it = this->_serv->_serverNames.begin();
	std::list<std::string>::iterator ite = this->_serv->_serverNames.end();

	if (it == ite)
		return ;

	std::string names = *it;
	it++;
	for (; it != ite; it++)
	{
		names += ':' + *it;
	}
	env_map.insert(std::pair<std::string, std::string>("SERVER_NAME", names));
}

void Cgi::addHeaderField(std::map<std::string, std::string> & env_map, std::string key, std::string header_key)
{
	size_t index_start = this->_header.find(header_key);

	if (index_start == std::string::npos)
		return ;

	size_t index_end = this->_header.find('\n', index_start);


	env_map.insert(std::pair<std::string, std::string>(key, this->_header.substr(index_start + header_key.size(), index_end - (index_start + header_key.size()) - (this->_header[index_end - 1] == '\r'))));
}
