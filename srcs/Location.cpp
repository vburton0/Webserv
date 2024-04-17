// Tout Copy Paste TO CHANGEEEEEEEEEEE /////////////////////////////

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
				std::cout << "LOCATION: " << this->location << std::endl;
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

void Location::trimSpaceAndSemicolon(std::string& str) {
    size_t semicolonPos = str.find(';');
    if (semicolonPos != std::string::npos) {
        str.erase(semicolonPos);
    }
    str.erase(str.find_last_not_of(" \t\n\r\f\v") + 1);
}

void Location::addToList(std::string& line, std::list<std::string>& list) {
    std::istringstream iss(line);
    std::string item;
    while (iss >> item) {
        if (item.back() == ';') {
            item.pop_back();
            if (item.empty()) {
                throw Webserv::InvalidFileContentException();
            }
        }
        list.push_back(item);
    }
    if (list.empty()) {
        throw Webserv::InvalidFileContentException();
    }
}

void Location::compareBlockInfo(std::string line) {
    std::cout << "line: " << line << std::endl;
    if (line.empty() || line[0] == '#')
        return;

    // Remove comments and trim line
    size_t commentPos = line.find('#');
    if (commentPos != std::string::npos) {
        line.erase(commentPos);
    }
    trimSpaceAndSemicolon(line);

    size_t spacePos = line.find(' ');
    if (spacePos == std::string::npos) {
        throw Webserv::InvalidFileContentException();
    }

    std::string directive = line.substr(0, spacePos);
    std::string content = line.substr(spacePos + 1);

    if (directive == "listen" || directive == "server_name" || directive == "root" || directive == "index" ||
        directive == "cgi" || directive == "upload_path" || directive == "return") {
        std::cout << directive << ": " << content << std::endl;
    } else if (directive == "client_max_body_size") {
        std::istringstream iss(content);
        std::string sizeStr;
        char unit;
        if (!(iss >> sizeStr >> unit) || (unit != 'M' && unit != 'K' && unit != ';')) {
            throw Webserv::InvalidFileContentException();
        }
        this->bodySize = std::stoul(sizeStr) * (unit == 'M' ? 1024 * 1024 : 1024);
    // } else if (directive == "error_page") {
    //     this->errorPages.push_back(content);
    } else if (directive == "allow_methods") {
        std::istringstream iss(content);
        std::string method;
        while (iss >> method) {
            this->methods.push_back(method);
        }
    } else if (directive == "directory_listing" || directive == "autoindex") {
        autoIndex = (content == "on");
    }else {
        throw Webserv::InvalidFileContentException(); // Unknown directive
    }
}


void Location::checkSetDefault(void) {
    if (!_return.empty()) {
        return;
    }
    if (root.empty()) {
        throw Webserv::InvalidFileContentException();
    }
    static const std::set<std::string> validMethods = {"GET", "PUT", "POST", "DELETE", "HEAD"};
    std::set<std::string> seenMethods;

    for (const auto& method : methods) {
        // Check for duplicate and validate method
        if (!seenMethods.insert(method).second || validMethods.find(method) == validMethods.end()) {
            throw Webserv::InvalidFileContentException();
        }
    }

    // Set default methods if none are provided
    if (methods.empty()) {
        methods = {"GET", "PUT", "POST", "DELETE", "HEAD"};
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
