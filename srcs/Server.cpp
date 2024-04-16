#include "../includes/Server.hpp"

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

// Wacthout ///////////////////////////////
void Server::checkSetDefault(void)
{
	if (this->ports.empty() || this->_root.empty())
		throw Server::IncompleteServerException();
	if (this->_errorPages.find(404) == this->_errorPages.end())
	{
		std::string err404("error/404.html");
		std::string file_abs_path = this->_root + err404;
		std::ifstream newdata(file_abs_path.c_str());
		if (!newdata.is_open())
			throw Webserv::MissingDefault404Exception();
		newdata.close();
		this->_errorPages.insert(std::pair<int,std::string>(404, err404));
	}
	if (this->_serverType.empty())
		this->_serverType = "default_server";
	for (size_t index = 0; index < this->_locations.size(); index ++)
	{
		for (size_t sub_index = 0; sub_index < index; sub_index++)
		{
			if (this->_locations[index]->location == this->_locations[sub_index]->location)
				throw Webserv::InvalidFileContentException();
		}
		if (!this->_locations[index]->_return.empty())
		{
			for (size_t loc_index = 0; loc_index < index; loc_index++)
			{
				if (this->_locations[loc_index]->location == this->_locations[index]->_return)
					*this->_locations[index] = *this->_locations[loc_index];
			}
			if (!this->_locations[index]->_return.empty())
				throw Webserv::InvalidFileContentException();
		}
	}
}

void Server::compareBlockInfo(std::string line, std::ifstream & indata)
{
	std::cout << "line: " << line << std::endl;
	if (!line.compare (0, 9, "location "))
		this->_locations.push_back(new Location(line, indata, this->_root));
	else if (line[line.size() - 1] != ';' || line.find(';') != line.size() - 1)
		throw Webserv::InvalidFileContentException();
	else if (!line.compare(0, 7, "listen "))
	{
		std::istringstream iss(line.substr(7));
		int toint;
		iss >> toint;
		if (iss.fail() || !toint)
			throw Webserv::InvalidFileContentException();
		this->ports.push_back(toint);
		size_t index = 7;
		while (std::isdigit(line[index]))
			++index;
		if (index == line.size() - 1 || (index == line.size() - 2 && line[index] == ' '))
			return ;
		if (line.compare(index, 15, " default_server"))
			throw Webserv::InvalidFileContentException();
		this->_serverType = "default_server";
	}
	else if (!line.compare(0, 12, "server_name "))
	{
		line = line.substr(12);
		if (line.empty() || !line.compare(";"))
			throw Webserv::InvalidFileContentException();
		while (!line.empty())
		{
			size_t size = line.find(' ');
			if (size == std::string::npos)
			{
				if (line.compare(";"))
					this->_serverNames.push_back(line.substr(0, line.size() - 1));
				line = "";
			}
			else
			{
				this->_serverNames.push_back(line.substr(0, size));
				line = line.substr(size + 1);
			}
		}
	}
	else if (!line.compare(0, 5, "root "))
	{
		this->_root = line.substr(5, line.size() - 6 - (line[line.size() - 2] == ' '));
		if (this->_root.find(' ') != std::string::npos || !this->_root.compare(";"))
			throw Webserv::InvalidFileContentException();
	}
	else if (!line.compare(0, 6, "index "))
	{
		line = line.substr(6);
		if (line.empty() || !line.compare(";"))
			throw Webserv::InvalidFileContentException();
		while (!line.empty())
		{
			size_t size = line.find(' ');
			if (size == std::string::npos)
			{
				if (line.compare(";"))
					this->_indexFile.push_back(line.substr(0, line.size() - 1));
				line = "";
			}
			else
			{
				this->_indexFile.push_back(line.substr(0, size));
				line = line.substr(size + 1);
			}
		}
	}
	else if (!line.compare("client_max_body_size 0;") || !line.compare("client_max_body_size 0 ;"))
	{
		if (this->_bodySighted)
			throw Webserv::InvalidFileContentException();
		this->_bodySighted = true;
		this->_maxBodySize = 0;
	}
	else if (!line.compare(0, 21, "client_max_body_size "))
	{
		if (this->_bodySighted)
			throw Webserv::InvalidFileContentException();
		this->_bodySighted = true;
		std::istringstream iss(line.substr(21));
		int toint;
		iss >> toint;
		if (iss.fail() || !toint)
			throw Webserv::InvalidFileContentException();
		this->_maxBodySize = toint;
		size_t index = 21;
		while (std::isdigit(line[index]))
			++index;
		if ((index == line.size() - 2 && line[index] == 'M') || (index == line.size() - 3 && line[index] == 'M' && line[index + 1] == ' '))
			return ;
		throw Webserv::InvalidFileContentException();
	}
	else if (!line.compare(0, 11, "error_page "))
	{
		std::list<int> toints;
		size_t index = 11;
		
		while (std::isdigit(line[index]))
		{
			std::istringstream iss(line.substr(index));
			int toint;
			iss >> toint;
			if (iss.fail())
				throw Webserv::InvalidFileContentException();
			if (!isErrorCode(toint))
				break ;
			toints.push_back(toint);
			while (std::isdigit(line[index]))
				++index;
			if (line[index] != ' ')
				throw Webserv::InvalidFileContentException();
			++index;
		}
		if (index == 11)
			throw Webserv::InvalidFileContentException();
		std::string value = line.substr(index, line.size() - index - 1 - (line[line.size() - 2] == ' '));
		if (value.find(' ') != std::string::npos)
			throw Webserv::InvalidFileContentException();
		std::string file_abs_path = this->_root + value;
		std::ifstream newdata(file_abs_path.c_str());
		if (!newdata.is_open())
			throw Webserv::InvalidFileContentException();
		newdata.close();
		std::list<int>::iterator it = toints.begin();
		std::list<int>::iterator ite = toints.end();
		for (; it != ite; it++)
			this->_errorPages.insert(std::pair<int,std::string>(*it, value));
	}
}

void Server::addPorts(std::set<int> &all_ports, size_t *number_of_ports) // Don't understand the point of this
{
	*number_of_ports += this->ports.size();
	std::list<int>::iterator it = this->ports.begin();
	std::list<int>::iterator ite = this->ports.end();
	for (; it != ite; it++)
	{
		if (all_ports.find(*it) != all_ports.end())
			it = this->ports.erase(it);
		all_ports.insert(*it);
	}
}

void Server::showServerContent(void) // Watchout All the iterator should be simplified
{
	std::cout << "Server: "
		<< "host: " << this->host << " port: " << this->port << " root: " << this->_root << std::endl;
	std::cout << "Server names: ";
	std::list<std::string>::iterator it = this->_serverNames.begin();
	std::list<std::string>::iterator ite = this->_serverNames.end();
	for (; it != ite; it++)
		std::cout << *it << " ";
	std::cout << std::endl;
	std::cout << "Ports: ";
	std::list<int>::iterator it2 = this->ports.begin();
	std::list<int>::iterator ite2 = this->ports.end();
	for (; it2 != ite2; it2++)
		std::cout << *it2 << " ";
	std::cout << std::endl;
	std::cout << "Error pages: ";
	std::map<int, std::string>::iterator it3 = this->_errorPages.begin();
	std::map<int, std::string>::iterator ite3 = this->_errorPages.end();
	for (; it3 != ite3; it3++)
		std::cout << it3->first << " " << it3->second << " ";
	std::cout << std::endl;
	std::cout << "Max body size: " << this->_maxBodySize << std::endl;
	std::cout << "Server type: " << this->_serverType << std::endl;
	std::cout << "Index file: ";
	std::list<std::string>::iterator it4 = this->_indexFile.begin();
	std::list<std::string>::iterator ite4 = this->_indexFile.end();
	for (; it4 != ite4; it4++)
		std::cout << *it4 << " ";
	std::cout << std::endl;
	std::cout << "Locations: " << std::endl;
	std::vector<Location *>::iterator it5 = this->_locations.begin();
	std::vector<Location *>::iterator ite5 = this->_locations.end();
	for (; it5 != ite5; it5++)
		(*it5)->showLocationContent();
}

std::string Server::recvRequest(int check_header)
{
	// std::string bufstr;
	// char buffer[BUFFER_SIZE + 1] = {0};
	// ssize_t valread = recv(this->socketFd, buffer, BUFFER_SIZE, 0);
	// if (valread == -1)
	// 	sendError(500, "500 Internal Server Error");
	// bufstr += buffer;
	// while (valread == BUFFER_SIZE) //TODO what if request of exactly BUFFER_SIZE bytes
	// {
	// 	sendResponse("100 Continue");
	// 	valread = recv(this->socketFd, buffer, BUFFER_SIZE, 0);
	// 	std::cout << "\n\n\nnvalread: \n\n\n\n" << valread << std::endl;
	// 	if (valread == -1)
	// 		sendError(500, "500 Internal Server Error");
	// 	else if (valread)
	// 	{
	// 		buffer[valread] = '\0';
	// 		bufstr += buffer;
	// 	}
	// }
	// if (check_header)
	// 	bufstr = checkChunckEncoding(bufstr);
	// return (bufstr);

	std::string request;
    char buffer[4096]; // Consider using a buffer size that fits your needs
    ssize_t nread;
    size_t content_length = 0; // You'll need to parse this from the headers

    // Initial read to get headers and possibly part of the body
    nread = recv(this->socketFd, buffer, sizeof(buffer), 0);
    if (nread > 0) {
        request.append(buffer, nread);
        // Parse headers to find Content-Length and set content_length accordingly
        // This is a simplified approach; you'll need to implement actual header parsing
        size_t pos = request.find("Content-Length: ");
        if (pos != std::string::npos) {
            size_t end = request.find("\r\n", pos);
            std::string cl = request.substr(pos + 16, end - (pos + 16));
            content_length = std::stoi(cl);
        }
    }

    // Keep reading until all data is received based on Content-Length
    while (request.length() < content_length && (nread = recv(this->socketFd, buffer, sizeof(buffer), 0)) > 0) {
        request.append(buffer, nread);
    }

	if (check_header)
		request = checkChunckEncoding(request);
    return request;
}


void Server::sendResponse(std::string msg)
{
	std::cerr << std::endl << " -- status return " << msg << " --" << std::endl;
	std::string content = "HTTP/1.1 " + msg + "\n\n";
	send(this->socketFd, content.c_str(), content.size(), 0);
}

void Server::sendError(int err_code, std::string errstr)
{
	std::cerr << std::endl << " -- status return " << errstr << " --" << std::endl;
	std::string content = "HTTP/1.1 " + errstr + '\n';
	if (this->_errorPages.find(err_code) != this->_errorPages.end())
	{
		std::string file_abs_path = this->_root + this->_errorPages[err_code];
		std::ifstream newdata(file_abs_path.c_str());
		if (!newdata.is_open())
			goto SEND;
		std::string file_content = readData(newdata);
		std::ostringstream content_length;

		content_length << file_content.size();
		content += "Content-Type:text/html\nContent-Length: ";
		content += content_length.str() + "\n\n" + file_content;
		newdata.close();
	}
	else
		content += '\n';
	SEND:
	send(this->socketFd, content.c_str(), content.size(), 0);
	throw Webserv::QuickerReturnException();
}

std::string Server::checkChunckEncoding(std::string bufstr)
{
	if (bufstr.find("Transfer-Encoding: chunked") == std::string::npos)
		return  (bufstr);
	std::string sub_bufstr;
	char buffer[BUFFER_SIZE + 1] = {0};
	// sendError(this->socketFd, 100, "100 Continue");
	ssize_t valread = recv(this->socketFd, buffer, BUFFER_SIZE, 0);
	if (valread == -1)
		sendError(500, "500 Internal Server Error");
	while (valread && buffer[0] != '0')
	{
		bufstr += buffer;
		displaySpecialCharacters(buffer);
		// sendError(this->socketFd, 100, "100 Continue");
		valread = recv(this->socketFd, buffer, BUFFER_SIZE, 0);
		if (valread == -1)
			sendError(500, "500 Internal Server Error");
		buffer[valread] = '\0';
		std::cout << "first char of buffer: |" << buffer[0] << '|' << std::endl;
	}
	return (bufstr);
}


void Server::analyseRequest(std::string bufstr)
{
	if (bufstr.empty())
	{
		std::cout << " -- empty request --" << std::endl;
		return ;
	}
	std::string content;
	// std::cout << bufstr.size() << ": " << bufstr << std::endl;
	displaySpecialCharacters(bufstr); //used for debug because /r present in buffer

	if (checkHttpVersion(bufstr))
		sendError(505, "505 HTTP Version Not Supported");
	if (checkCorrectHost(bufstr, this->_serverNames) || checkHTTPHeaderFormat(bufstr))
		sendError(400, "400 Bad Request");
	if (!bufstr.compare(0, 4, "GET ") || !bufstr.compare(0, 5, "HEAD "))
	{
		int head_offset = !bufstr.compare(0, 5, "HEAD ");
		std::cout << "GET DETECTED" << std::endl;
		std::string file_abs_path = getPathFromLocations(bufstr, head_offset, "GET", 0);

		if (file_abs_path == "index.html")
			file_abs_path = this->_root + file_abs_path;
		size_t qmark = file_abs_path.find('?');
		if (qmark != std::string::npos)
			file_abs_path = file_abs_path.substr(0, qmark);

		std::cout << "reading from file: |" << file_abs_path << '|' << std::endl;
		std::ifstream indata(file_abs_path.c_str());
		if (!indata.is_open())
			sendError(404, "404 Not Found");
		
		DIR *dir = opendir(file_abs_path.c_str());
		if (dir != NULL)
			dirListing(dir);

		content = "HTTP/1.1 200 OK\nContent-Type: " + GetContentType(file_abs_path) + "\nContent-Length: ";
		std::string file_content = readData(indata);

		std::ostringstream content_length;
		content_length << file_content.size();
		content += content_length.str() + "\n\n" + file_content;
		send(this->socketFd, content.c_str(), content.size(), 0);
	}
	else if (!bufstr.compare(0, 4, "PUT ") || !bufstr.compare(0, 5, "POST "))
	{
		int post_offset = !bufstr.compare(0, 5, "POST ");

		if (!post_offset)
			std::cout << "PUT DETECTED" << std::endl;
		else
			std::cout << "POST DETECTED" << std::endl;

		std::string file_abs_path;
		if (!post_offset)
			file_abs_path = getPathFromLocations(bufstr, 0, "PUT", 0);
		else
			file_abs_path = getPathFromLocations(bufstr, 1, "POST", 0);

		std::string line;
		size_t index = bufstr.find("Content-Length: ");
		if (index == std::string::npos)
			sendError(411, "411 Length Required");
		std::istringstream iss(bufstr.substr(index + 16, bufstr.find('\n', index + 16)));
		size_t expected_size;
		iss >> expected_size;
		if (iss.fail() || expected_size > this->actualBodySize * 1000000)
			sendError(412, "412 Precondition Failed");

		std::cout << "checking if file: |" << file_abs_path << "| exists" << std::endl;
		std::ifstream indata(file_abs_path.c_str());
		if (!indata.is_open())
			content = "201 Created";
		else
			content = "200 OK";
		indata.close();
		if (!post_offset)
		{
			std::ofstream outdata(file_abs_path.c_str(), std::ofstream::trunc);
			if (!outdata.is_open())
				sendError(404, "404 Not Found");
			std::string body =  getBody(bufstr);
			if (body.empty() && expected_size)
			{
				sendResponse("100 Continue");
				body = recvRequest(0);	
			}
			handleRequest(body, outdata, expected_size, content);
			outdata.close();
		}
		else
		{
			std::string body = getBody(bufstr);
			if (body.size() != expected_size)
				sendError(412, "412 Precondition Failed");
			else if (expected_size > this->actualBodySize * 1000000)
			{
				std::cerr << "file size exceeds max_body_size of " << this->actualBodySize << "MB" << std::endl;
				sendError(413, "413 Payload Too Large");
			}
			body = "Body received :\n" + body;
			content = "HTTP/1.1 202 Accepted\nContent-Type: text/plain\nContent-Length: ";
			std::ostringstream content_length;
			content_length << body.size();
			content += content_length.str() + "\n\n" + body;
			send(this->socketFd, content.c_str(), content.size(), 0);
			std::cout << "-- return status 202 Accepted --" << std::endl;
		}
	}
	else if (!bufstr.compare(0, 7, "DELETE "))
	{
		std::cout << "DELETE DETECTED" << std::endl;
		std::string file_abs_path = getPathFromLocations(bufstr, 3, "DELETE", 0);
		std::cout << "Wants to delete file " << file_abs_path << std::endl;
		if (!std::remove(file_abs_path.c_str()))
		{
			std::cout << "File deleted\n";
			content = "HTTP/1.1 204 No Content\n";
		}
		else
		{
			std::cout << "File could not be deleted\n";
			content = "HTTP/1.1 404 Not Found\n";
		}
		send(this->socketFd, content.c_str(), content.size(), 0);
	}
}


std::string Server::getPathFromLocations(std::string & loc, int methodOffset, std::string method, bool recursive_stop)
{
	std::string ret;
	std::string substr_loc = loc.substr(4 + methodOffset, loc.find(" ", 4 + methodOffset) - (4 + methodOffset));
	size_t match_size = 0;
	size_t match_index;
	bool auto_index;
	std::list<std::string> match_index_files;
	for (size_t loc_index = 0; loc_index < this->_locations.size(); loc_index++)
	{
		size_t loc_size = this->_locations[loc_index]->location.size();
		if (loc_size <= substr_loc.size() && ((!this->_locations[loc_index]->suffixed && !substr_loc.compare(0, loc_size, this->_locations[loc_index]->location))
			|| (this->_locations[loc_index]->suffixed && !substr_loc.compare(substr_loc.size() - loc_size, loc_size, this->_locations[loc_index]->location))))
		{
			if (this->_locations[loc_index]->location[this->_locations[loc_index]->location.size() - 1] != '/' && loc_size < substr_loc.size() && substr_loc[loc_size] != '/')
				continue ;
			if (!match_size || loc_size > match_size || this->_locations[loc_index]->suffixed)
			{
				match_size = loc_size;
				match_index = loc_index;
				match_index_files = this->_locations[loc_index]->index_files;
				auto_index = this->_locations[loc_index]->auto_index;
				this->actualBodySize = this->_locations[loc_index]->body_size;
				if (this->_locations[loc_index]->suffixed)
				{
					match_size = 1;
					break ;
				}
			}
		}
	}
	if (!recursive_stop && match_size && std::find(this->_locations[match_index]->methods.begin(), this->_locations[match_index]->methods.end(), method) == this->_locations[match_index]->methods.end())
	{
		sendErrorMethod(this->_locations[match_index]->methods);
		throw Webserv::QuickerReturnException();
	}
	if (this->actualBodySize == std::string::npos)
		this->actualBodySize = this->_maxBodySize;
	if (!match_size)
		ret = this->_root;
	else
		ret = this->_locations[match_index]->root;
	this->_initial_loc = substr_loc;
	std::string savedRoot = ret;
	loc = loc.substr(0, 4 + methodOffset) + loc.substr(4 + methodOffset + match_size);
	if (!loc.compare(4 + methodOffset, 2, "/ ") || !loc.compare(4 + methodOffset, 1, " "))
	{
		if (match_index_files.empty())
			ret = getFirstIndexFile(ret, this->_indexFile, auto_index && !method.compare("GET"));
		else
			ret = getFirstIndexFile(ret, match_index_files, auto_index && !method.compare("GET"));
	}
	else if (!loc.compare(4 + methodOffset, 1, "/"))
		ret += loc.substr(5 + methodOffset, loc.find(" ", 5 + methodOffset) - (5 + methodOffset));
	else
		ret += loc.substr(4 + methodOffset, loc.find(" ", 4 + methodOffset) - (4 + methodOffset));
	loc = loc.substr(0, 4 + methodOffset) + ret + loc.substr(loc.find(' ', 4 + methodOffset));
	if (recursive_stop)
		return ("");
	ret = checkForCGI(loc, ret, 4 + methodOffset, method, savedRoot);
	if (match_size && !this->_locations[match_index]->cgi.empty())
		Cgi(loc, this->_locations[match_index]->root + this->_locations[match_index]->cgi, this, savedRoot);
	return (ret);
}

void Server::dirListing(DIR *dir)
{
	std::string body = "<!DOCTYPE html>\n<html>\n <body>\n<center>\n	<div>\n		<H1>Index of " + getLastWord(this->_initial_loc) + "</H1>\n	</div>\n";
	struct dirent *dent;
	std::string dot = ".";

	while ((dent = readdir(dir)) != NULL)
	{
		if (dot.compare(0, 2, dent->d_name))
		{
			body += "<p><a href=\"" + this->_initial_loc;
			if (this->_initial_loc[this->_initial_loc.size() - 1] != '/')
				body += '/';
			body += std::string(dent->d_name) + "\">" + std::string(dent->d_name) + "</a></p>\n";
		}
	}
	closedir(dir);
	body += "</center>\n </body>\n</html>";
	std::ostringstream content_length;
	content_length << body.size();

	std::string content = "HTTP/1.1 200 OK\nContent-Type: text/html\nContent-Length: ";
	content += content_length.str() + "\n\n" + body;
	send(this->socketFd, content.c_str(), content.size(), 0);
	throw Webserv::QuickerReturnException();
}

void Server::sendErrorMethod(std::vector<std::string> methods)
{
	std::string content = "405 Method Not Allowed\nAllow: ";
	std::vector<std::string>::iterator it = methods.begin();
	std::vector<std::string>::iterator ite = methods.end();
	for (; it != ite; it++)
	{
		content += *it;
		if (++it != ite)
			content += ", ";
		--it;
	}
	sendError(405, content);
}

void Server::handleRequest(std::string body, std::ofstream &outfile, size_t expected_size, std::string content)
{
	std::cout << "body size: " << body.size() << ", selected max_body_size: " << this->actualBodySize << std::endl;
	// std::cout << body << std::endl;

	if (body.size() > this->actualBodySize * 1000000)
	{
		std::cerr << "file size exceeds max_body_size of " << this->actualBodySize << "MB" << std::endl;
		sendError(413, "413 Payload Too Large");
	}
	else if (body.size() != expected_size)
		sendError(412, "412 Precondition Failed");

	outfile << body;
	sendResponse(content);
}

std::string Server::getFirstIndexFile(std::string root, std::list<std::string> index_files, bool auto_index)
{
	std::list<std::string>::iterator it = index_files.begin();
	std::list<std::string>::iterator ite = index_files.end();
	for (; it != ite; it++)
	{
		std::string file_abs_path = root + *it;
		std::ifstream newdata(file_abs_path.c_str());
		if (newdata.is_open())
		{
			newdata.close();
			return (*it);
		}
	}
	if (auto_index)
	{
		DIR *dir = opendir(root.c_str());
		if (dir != NULL)
		{
			closedir(dir);
			return ("");
		}
	}
	sendError(404, "404 Not Found");
	return ("");
}



/// end WATCHOUT ////////////////////////////////////////////////
/* look for "/cgi/" in filePath and call cgi if found */
std::string Server::checkForCGI(std::string header, std::string filePath, int methodOffset, std::string method, std::string savedRoot)
{
	size_t cgiPos = filePath.find("/cgi/");
    if(cgiPos == std::string::npos) {
    	return filePath; 
	}
	
	size_t uploadedPos = filePath.find("/uploadedFiles/");
    if (uploadedPos != std::string::npos) {
        return filePath.erase(cgiPos, 4);  // Remove the "/cgi" part from the path
    }


	size_t end = filePath.find('/', cgiPos + 5);
	if (end == std::string::npos)
	{
		size_t end_mark = filePath.find('?', cgiPos + 5);
		if (end_mark == std::string::npos)
			header = header.substr(0, methodOffset) + '/' + header.substr(methodOffset + filePath.size());
		else
		{
			filePath = filePath.substr(0, end_mark);
			header = header.substr(0, methodOffset) + header.substr(methodOffset + end_mark);
		}
	}
	else
	{
		filePath = filePath.substr(0, end);
		header = header.substr(0, methodOffset) + header.substr(methodOffset + end);
	}
	getPathFromLocations(header, methodOffset - 4, method, 1);
	Cgi(header, filePath, this, savedRoot);
	return filePath;
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

const char* Server::IncompleteServerException::what() const throw()
{
	return ("[Server::IncompleteServerException] Missing line in server block.");
}
