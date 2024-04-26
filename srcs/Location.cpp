#include "../includes/Location.hpp"

Location::Location(std::string line, std::ifstream & openFile, std::string root) : _autoSighted(false),
	_lineSighted(false), _returnSighted(false), _bodySighted(false), root(root), bodySize(std::string::npos),
	autoIndex(true), suffixed(false)
{
	if (root.empty())
		throw Webserv::InvalidFileContentException();
	size_t index = line.find(' ', 9);
	if (line.compare(index + 1, 2, "{") && line[9] != '~')
		throw Webserv::InvalidFileContentException();
	this->location = line.substr(9, line.find(' ', 9) - 9);
	if (this->location[0] != '/' && this->location.compare(0, 2, "~"))
		throw Webserv::InvalidFileContentException();
	if (!this->location.compare(0, 2, "~"))
	{
		this->suffixed = true;
		index = line.find(' ', 11);
		if (line.compare(index + 1, 2, "{"))
			throw Webserv::InvalidFileContentException();
		this->location = line.substr(11, line.find(' ', 11) - 11);
		if (this->location[0] != '.' || !this->location.compare(0, 2, "."))
			throw Webserv::InvalidFileContentException();
	}
	while (!openFile.eof())
	{
		std::getline(openFile, line);
		line = trimSpaceComments(line);
		if (!line.empty())
		{
			if (!line.compare("}"))
			{
				checkSetDefault();
				return ;
			}
			else
			{
				if (this->_returnSighted)
					throw Webserv::InvalidFileContentException();
				compareBlockInfo(line);
				this->_lineSighted = true;
			}
		}
	}
	throw Webserv::InvalidFileContentException();
}

Location::~Location(void)
{

}

Location &Location::operator=(const Location & src)
{
	this->_return = src._return;
	this->methods = src.methods;
	this->autoIndex = src.autoIndex;
	this->root = src.root;
	this->indexFiles = src.indexFiles;
	this->bodySize = src.bodySize;
	this->cgi = src.cgi;
	return (*this);
}

void Location::compareBlockInfo(std::string line)
{
	if (line[0] == '#')
		;
	else if (line[line.size() - 1] != ';' || line.find(';') != line.size() - 1)
		throw Webserv::InvalidFileContentException();
	else if (!line.compare(0, 5, "root "))
	{
		this->root = line.substr(5, line.size() - 6 - (line[line.size() - 2] == ' '));
		if (this->root.find(' ') != std::string::npos || !this->root.compare(";"))
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
					this->indexFiles.push_back(line.substr(0, line.size() - 1));
				line = "";
			}
			else
			{
				this->indexFiles.push_back(line.substr(0, size));
				line = line.substr(size + 1);
			}
		}
	}
	else if (!line.compare("client_max_body_size 0;") || !line.compare("client_max_body_size 0 ;"))
	{
		if (this->_bodySighted)
			throw Webserv::InvalidFileContentException();
		this->_bodySighted = true;
		this->bodySize = 0;
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
		this->bodySize = toint;
		size_t index = 21;
		while (std::isdigit(line[index]))
			++index;
		if ((index == line.size() - 2 && line[index] == 'M') || (index == line.size() - 3 && line[index] == 'M' && line[index + 1] == ' '))
			return ;
		throw Webserv::InvalidFileContentException();
	}
	else if (!line.compare(0, 10, "autoindex "))
	{
		if (this->_autoSighted)
			throw Webserv::InvalidFileContentException();
		this->_autoSighted = true;
		line = line.substr(10, line.size() - 11 - (line[line.size() - 2] == ' '));
		if (line.find(' ') != std::string::npos || !line.compare(";"))
			throw Webserv::InvalidFileContentException();
		if (!line.compare("on"))
			this->autoIndex = true;
		else if (!line.compare("off"))
			this->autoIndex = false;
		else
			throw Webserv::InvalidFileContentException();
	}
	else if (!line.compare(0, 14, "allow_methods "))
	{
		line = line.substr(14);
		if (line.empty() || !line.compare(";"))
			throw Webserv::InvalidFileContentException();
		while (!line.empty())
		{
			size_t size = line.find(' ');
			if (size == std::string::npos)
			{
				if (line.compare(";"))
					this->methods.push_back(line.substr(0, line.size() - 1));
				line = "";
			}
			else
			{
				this->methods.push_back(line.substr(0, size));
				line = line.substr(size + 1);
			}
		}
	}
	else if (!line.compare(0, 4, "cgi "))
	{
		if (!this->cgi.empty())
			throw Webserv::InvalidFileContentException();
		line = line.substr(4, line.size() - 5 - (line[line.size() - 2] == ' '));
		if (line.find(' ') != std::string::npos || !line.compare(";"))
			throw Webserv::InvalidFileContentException();
		this->cgi = line;
	}
	else if (!line.compare(0, 7, "return "))
	{
		if (this->_lineSighted)
			throw Webserv::InvalidFileContentException();
		this->_return = line.substr(7, line.size() - 8 - (line[line.size() - 2] == ' '));
		if (this->_return.find(' ') != std::string::npos || !this->_return.compare(";"))
			throw Webserv::InvalidFileContentException();
		this->_returnSighted = true;
	}
}

std::set<std::string> Location::createValidMethodsSet() {
        std::set<std::string> methods;
        methods.insert("GET");
        methods.insert("PUT");
        methods.insert("POST");
        methods.insert("DELETE");
        methods.insert("HEAD");
        return methods;
}

void Location::checkSetDefault(void) {
    if (!_return.empty()) {
        return;
    }
    if (root.empty()) {
        throw Webserv::InvalidFileContentException();
    }
    static const std::set<std::string> validMethods = createValidMethodsSet();
    std::set<std::string> seenMethods;

    for (std::vector<std::string>::const_iterator it = methods.begin(); it != methods.end(); ++it) {
            std::pair<std::set<std::string>::iterator, bool> result = seenMethods.insert(*it);
            if (!result.second || validMethods.find(*it) == validMethods.end()) {
            	throw Webserv::InvalidFileContentException();
        }
    }

    if (methods.empty()) {
        for (std::set<std::string>::const_iterator it = validMethods.begin(); it != validMethods.end(); ++it) {
        	methods.push_back(*it);
    	}
    } else {
        std::sort(methods.begin(), methods.end());
    }
}


void Location::showLocationContent(void) {
    std::cout << "\t-location: " << location << std::endl;
    std::cout << "\t  -root: " << root << std::endl;
    std::cout << "\t  -autoIndex: " << (autoIndex ? "on" : "off") << std::endl;
    std::cout << "\t  -index: ";
    printList(indexFiles);
    std::cout << std::endl;
    if (bodySize != std::string::npos) {
        std::cout << "\t  -bodySize: " << bodySize << std::endl;
    }
    std::cout << "\t  -methods: ";
    printList(methods);
    std::cout << std::endl;
    std::cout << "\t  -suffixed: " << suffixed << std::endl;
    std::cout << "\t  -cgi: " << cgi << std::endl;
}
