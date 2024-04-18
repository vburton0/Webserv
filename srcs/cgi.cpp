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

		const size_t CHUNK_SIZE = 1024;
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
				exit(1); 
			}
		}
	}

	int pipe_fd[2];
	if (pipe(pipe_fd) == -1)
		serv->sendError(500, "500 Internal Server Error");
	pid_t pid = fork();
	if (pid == -1)
		serv->sendError(500, "500 Internal Server Error");
	if (!pid)
	{
		//Setup env
		char **envp = setEnv(saved_root);
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
	}
	close(pipe_fd[0]);
	waitpid(pid, NULL, 0);

	std::cout << "RETURN CGI:\n" + bufstr;


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

char** Cgi::getExecveArgs() {
    size_t size = _file_path.size();

    char** res = new char*[3];  // Interpreter, script path, NULL
    res[2] = NULL;

    // Check for Python scripts
    if (size > 3 && _file_path.compare(size - 3, 3, ".py") == 0) {
        res[0] = strdup("/usr/bin/python3");
        res[1] = strdup(_file_path.c_str());
        return res;
    }

    delete[] res;
    res = new char*[2]; // Only path and NULL
    res[0] = strdup(_file_path.c_str());
    res[1] = NULL;
    return res;
}

char **Cgi::setEnv(std::string saved_root)
{
	std::map<std::string, std::string> env_map;
	env_map["SERVER_PROTOCOL"] = "HTTP/1.1";
    env_map["SERVER_PORT"] = get_port();
    env_map["REQUEST_METHOD"] = getMethod();
    insertPathInfo(env_map, saved_root);
    env_map["SCRIPT_NAME"] = getScriptRelative(saved_root);
    env_map["REMOTE_HOST"] = getRemoteHost();
	addServerNames(env_map);
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
	size_t start = _header.find("Host: ");
	size_t end = _header.find('\n', start);

	std::string host_line = _header.substr(start, end - start);
	size_t index_port = host_line.find(':', 5);
	if (index_port == std::string::npos)
		return ("80");
	return (host_line.substr(index_port + 1, host_line.size() - (index_port + 1) - (host_line[host_line.size() - 1] == '\r')));
}

std::string Cgi::getMethod(void)
{
	size_t index = _header.find(' ');
	return (_header.substr(0, index));
}

void Cgi::insertPathInfo(std::map<std::string, std::string> & env_map, std::string root)
{
	size_t start = _header.find(' ') + 1;
	size_t end = _header.find(' ', start);
	size_t index_query = _header.find('?', start);
	if (index_query != std::string::npos && index_query < end)
	{
		std::string query_string = _header.substr(index_query + 1, end - (index_query + 1));
		env_map["QUERY_STRING"] = query_string;
		end = index_query;
	}
	std::string path_info = _header.substr(start + root.size(), end - (start + root.size()));
	std::string path_translated = _header.substr(start, end - start);
	env_map["PATH_INFO"] = path_info;
    env_map["PATH_TRANSLATED"] = path_translated;
}

std::string Cgi::getScriptRelative(std::string root)
{
	return (_file_path.substr(root.size(), _file_path.size() - root.size()));
}

std::string Cgi::getRemoteHost() {
    size_t start = _header.find("Host: ") + 6; // Skip past "Host: "
    size_t end = _header.find('\r', start); // Search for carriage return
    size_t index_port = _header.find(':', start);

    if (index_port != std::string::npos && index_port < end) {
        return _header.substr(start, index_port - start);
    }
    return _header.substr(start, end - start);
}

void Cgi::addServerNames(std::map<std::string, std::string> & env_map) {
    if (_serv->_serverNames.empty()) return;

    std::string names;
    for (std::list<std::string>::const_iterator it = _serv->_serverNames.begin(); it != _serv->_serverNames.end(); ++it) {
        if (!names.empty()) names += ':';
        names += *it;
    }
    env_map["SERVER_NAME"] = names;
}

void Cgi::addHeaderField(std::map<std::string, std::string> & env_map, std::string key, std::string header_key)
{
	size_t start = _header.find(header_key);

	if (start == std::string::npos)
		return ;

	start += header_key.size();
    size_t end = _header.find('\r', start);
    if (end == std::string::npos) return;


	env_map[key] = _header.substr(start, end - start);
}
