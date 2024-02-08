// Tout Copy Paste TO CHANGEEEEEEEEEEE /////////////////////////////

#include "../includes/Location.hpp"

Location::Location(std::string line, std::ifstream & onpenFile, std::string root) : _auto_sighted(false),
	_line_sighted(false), _return_sighted(false), _body_sighted(false), root(root), body_size(std::string::npos),
	auto_index(true), suffixed(false)
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
	while (!onpenFile.eof())
	{
		std::getline(onpenFile, line);
		line = trimSpaceComments(line);
		if (!line.empty())
		{
			if (!line.compare("}"))
			{
				check_set_default();
				std::cout << "LOCATION: " << this->location << std::endl;
				return ;
			}
			else
			{
				if (this->_return_sighted)
					throw Webserv::InvalidFileContentException();
				compare_block_info(line);
				this->_line_sighted = true;
			}
		}
	}
	throw Webserv::InvalidFileContentException();
}

Location::~Location(void)
{

}

Location &Location::operator=(const Location & other)
{
	this->_return = other._return;
	this->methods = other.methods;
	this->auto_index = other.auto_index;
	this->root = other.root;
	this->index_files = other.index_files;
	this->body_size = other.body_size;
	this->cgi = other.cgi;
	return (*this);
}

// ************************************************************************** //
//                                  Private                                   //
// ************************************************************************** //

void Location::compare_block_info(std::string line)
{
	std::cout << "line: " << line << std::endl;
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
					this->index_files.push_back(line.substr(0, line.size() - 1));
				line = "";
			}
			else
			{
				this->index_files.push_back(line.substr(0, size));
				line = line.substr(size + 1);
			}
		}
	}
	else if (!line.compare("client_max_body_size 0;") || !line.compare("client_max_body_size 0 ;"))
	{
		if (this->_body_sighted)
			throw Webserv::InvalidFileContentException();
		this->_body_sighted = true;
		this->body_size = 0;
	}
	else if (!line.compare(0, 21, "client_max_body_size "))
	{
		if (this->_body_sighted)
			throw Webserv::InvalidFileContentException();
		this->_body_sighted = true;
		std::istringstream iss(line.substr(21));
		int toint;
		iss >> toint;
		if (iss.fail() || !toint)
			throw Webserv::InvalidFileContentException();
		this->body_size = toint;
		size_t index = 21;
		while (std::isdigit(line[index]))
			++index;
		if ((index == line.size() - 2 && line[index] == 'M') || (index == line.size() - 3 && line[index] == 'M' && line[index + 1] == ' '))
			return ;
		throw Webserv::InvalidFileContentException();
	}
	else if (!line.compare(0, 10, "autoindex "))
	{
		if (this->_auto_sighted)
			throw Webserv::InvalidFileContentException();
		this->_auto_sighted = true;
		line = line.substr(10, line.size() - 11 - (line[line.size() - 2] == ' '));
		if (line.find(' ') != std::string::npos || !line.compare(";"))
			throw Webserv::InvalidFileContentException();
		if (!line.compare("on"))
			this->auto_index = true;
		else if (!line.compare("off"))
			this->auto_index = false;
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
		if (this->_line_sighted)
			throw Webserv::InvalidFileContentException();
		this->_return = line.substr(7, line.size() - 8 - (line[line.size() - 2] == ' '));
		if (this->_return.find(' ') != std::string::npos || !this->_return.compare(";"))
			throw Webserv::InvalidFileContentException();
		this->_return_sighted = true;
	}
}

void Location::check_set_default(void)
{
	if (!this->_return.empty())
		return ;
	if (this->root.empty())
		throw Webserv::InvalidFileContentException();
	for (size_t index = 0; index < this->methods.size(); index++)
	{
		for (size_t sub_index = 0; sub_index < index; sub_index++)
		{
			if (this->methods[index] == this->methods[sub_index])
				throw Webserv::InvalidFileContentException();
		}
		if (this->methods[index] != "GET" && this->methods[index] != "PUT" && this->methods[index] != "POST"
			&& this->methods[index] != "DELETE" && this->methods[index] != "HEAD")
			throw Webserv::InvalidFileContentException();
	}
	if (this->methods.empty())
	{
		this->methods.push_back("GET");
		this->methods.push_back("PUT");
		this->methods.push_back("POST");
		this->methods.push_back("DELETE");
		this->methods.push_back("HEAD");
	}
	std::sort(this->methods.begin(), this->methods.end());
}


// ************************************************************************** //
//                                  Public                                    //
// ************************************************************************** //

void Location::showLocationContent(void)
{
	std::cout << "\t-location: " << this->location << std::endl;
	std::cout << "\t  -root: " << this->root << std::endl;
	std::cout << "\t  -auto_index: ";
	(this->auto_index)
		? std::cout << "on" << std::endl
		: std::cout << "off" << std::endl;
	std::cout << "\t  -index: ";
	std::list<std::string>::iterator it = this->index_files.begin();
	std::list<std::string>::iterator ite = this->index_files.end();
	for (; it != ite; it++)
		std::cout << *it << ' ';
	std::cout << std::endl;
	if (this->body_size != std::string::npos)
		std::cout << "\t  -body_size: " << this->body_size << std::endl;
	std::cout << "\t  -methods: ";
	std::vector<std::string>::iterator iit = this->methods.begin();
	std::vector<std::string>::iterator iite = this->methods.end();
	for (; iit != iite; iit++)
		std::cout << *iit << ' ';
	std::cout << std::endl;
	std::cout << "\t  -suffixed: " << this->suffixed << std::endl;
	std::cout << "\t  -cgi: " << this->cgi << std::endl;
	std::cout << "\t  -return: " << this->_return << std::endl;
}
