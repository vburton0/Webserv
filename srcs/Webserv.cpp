#include "../includes/Webserv.hpp"
#include <fstream>

Webserv::Webserv() {}

Webserv::~Webserv() {}


void Webserv::init(std::string configFile) {
    // Check if configFile extension is ".conf"
    if (!hasExtension(configFile, ".conf")) {
        std::cerr << "Invalid config file extension. Must be '.conf'." << std::endl;
    }
    std::ifstream openFile(configFile.c_str());
    if (!openFile.is_open()) {
        throw std::runtime_error("Failed to open configuration file: " + configFile);
    }
    std::string line;
    bool inServerBlock = false;
    while (!openFile.eof()) {
        std::getline(openFile, line);
        line = trimSpaceComments(line);
        if (!line.empty())
		{
			if (!inServerBlock && !line.compare("server {"))
			{
				inServerBlock = true;
				this->servers.push_back(new Server());  // Add new server to list WACTHOUT
			}
			else if (inServerBlock)
			{
				if (!line.compare("}"))
				{
					inServerBlock = false;
					this->servers.back()->checkSetDefault();
				}
				else
					this->servers.back()->compareBlockInfo(line, openFile);
			}
			else if (line[0] != '#')
				throw Webserv::InvalidFileContentException();
		}
    }
	openFile.close();
	if (this->servers.empty())
		throw Webserv::InvalidFileContentException(); // No server block found //Watchout not same
	
	std::set<int> ports;
	for (std::list<Server *>::iterator it = this->servers.begin(); it != this->servers.end(); it++)
		(*it)->addPorts(ports);


}

void Webserv::showServersContent(void)
{
	for (std::list<Server *>::iterator it = this->servers.begin(); it != this->servers.end(); it++)
		(*it)->showServerContent();
}

void Webserv::launchServers(void)
{
	std::list<Server *>::iterator it = this-> servers.begin();
	std::list<Server *>::iterator ite = this-> servers.end();
	
	std::list<Server *>::iterator it_tmp = it;

	size_t total_ports = 0;
	for (; it_tmp != ite; it_tmp++)
		total_ports += (*it_tmp)->ports.size();  // Count total number of ports to listen on

	struct pollfd pfds[total_ports];
	struct sockaddr_in address[total_ports];
	size_t index = 0;

	for (; it != ite; it++)
	{
		if ((*it)->ports.empty())
			continue ;

		std::list<int>::iterator pit = (*it)->ports.begin();
		std::list<int>::iterator pite = (*it)->ports.end();
		
		for (; pit != pite; pit++)
		{
			std::cout << "Setting up port " << *pit << '.' << std::endl;

			if ((pfds[index].fd = socket(PF_INET, SOCK_STREAM, 0)) == 0)
			{
				perror("socket");
				return ;
			}
			pfds[index].events = POLLIN;

			address[index].sin_family = PF_INET;
			address[index].sin_addr.s_addr = INADDR_ANY;
			address[index].sin_port = htons(*pit);

			if (bind(pfds[index].fd, (struct sockaddr *) &address[index], sizeof(address[index])) < 0)
			{
				perror("bind");
				return ;
			}

			if (listen(pfds[index].fd, 10) < 0)
			{
				perror("listen");
				return ;
			}
			++index;
		}
	}

	int ready;

	while(1)
	{
		std::cout << "\n+++++++ Waiting for new connection ++++++++\n\n";
		ready = poll(pfds, total_ports, -1);
		if (ready == -1)
		{
			perror("poll");
			return ;
		}

		(ready == 1)
			? std::cout << "1 socket ready" << std::endl
			: std::cout << ready << " sockets ready" << std::endl;
		for (size_t index = 0; index < total_ports; index++) {

			if (pfds[index].revents != 0) {
				std::cout << "  - (index " << index << ") socket = " << pfds[index].fd << "; events: ";
				(pfds[index].revents & POLLIN)  ? std::cout << "POLLIN "  : std::cout << "";
				(pfds[index].revents & POLLHUP) ? std::cout << "POLLHUP " : std::cout << "";
				(pfds[index].revents & POLLERR) ? std::cout << "POLLERR " : std::cout << "";
				std::cout << std::endl << std::endl;

				Server *polling_serv = getPollingServer(ntohs(address[index].sin_port));
				if (!polling_serv)
				{
					std::cerr << "ERROR: no matching port in servers ?????" << std::endl;
					continue ;
				}

				// addrlen = sizeof(address[index]);
				if ((polling_serv->socketFd = accept(pfds[index].fd, NULL, 0)) < 0)
				{
					perror("accept");
					return ;
				}

				try
				{
					std::string bufstr = polling_serv->recvRequest(1);
					polling_serv->analyseRequest(bufstr);
				}
				catch (std::exception & e) {/*std::cerr << e.what() << std::endl;*/}
				std::cout << std::endl << "------------------content message sent to " << polling_serv->socketFd << "-------------------\n";
				close(polling_serv->socketFd);
			}
		}
	}
}

Server *Webserv::getPollingServer(uint16_t sin_port)
{
	std::list<Server *>::iterator it = this->servers.begin();
	std::list<Server *>::iterator ite = this->servers.end();
	for (; it != ite; it++)
	{
		std::list<int>::iterator pit = (*it)->ports.begin();
		std::list<int>::iterator pite = (*it)->ports.end();
		for (; pit != pite; pit++)
		{
			if (*pit == sin_port)
				return (*it);
		}
	}
	return (NULL);
}

























const char* Webserv::InvalidFileException::what() const throw()
{
	return ("[Webserv::InvalidFileException] Configuration file could not be opened.");
}

const char* Webserv::InvalidFileExtensionException::what() const throw()
{
	return ("[Webserv::InvalidFileExtensionException] Configuration file does not end with '.conf'.");
}

const char* Webserv::InvalidFileContentException::what() const throw()
{
	return ("[Webserv::InvalidFileContentException] Configuration file contains invalid server info.");
}

const char* Webserv::MissingDefault404Exception::what() const throw()
{
	return ("[Webserv::MissingDefault404Exception] No default 404 file in root provided by conf file.");
}

const char* Webserv::SystemCallException::what() const throw()
{
	return ("[Webserv::SystemCallException] System call did not call.");
}

const char* Webserv::QuickerReturnException::what() const throw()
{
	return ("[Webserv::QuickerReturnException] gota go fast.");
}

const char* Webserv::PortsException::what() const throw()
{
	return ("[Webserv::PortsException] You cannot have 2 same port.");
}
